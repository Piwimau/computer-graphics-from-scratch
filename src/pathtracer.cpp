#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <vector>
#include "cgfs/color.hpp"
#include "cgfs/pathtracer.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"
#include "dielectric-bsdf.hpp"
#include "medium-stack.hpp"
#include "microfacet-bsdf.hpp"
#include "prng.hpp"
#include "sampling.hpp"

namespace cgfs {

/** @brief Information about a scene suitable for path tracing. */
struct SceneInfo {

    /** @brief The materials in the scene. */
    std::vector<Material> materials;

    /**
     * @brief The spheres in the scene.
     *
     * @note This vector is ordered such that opaque spheres come before
     * transparent spheres.
     */
    std::vector<Sphere> spheres;

    /** @brief The number of opaque spheres in the scene. */
    isize numOpaqueSpheres;

    /** @brief The indices of the emissive spheres in the scene. */
    std::vector<isize> emissiveSpheresIndices;

    /** @brief The lights in the scene. */
    std::vector<Light> lights;

    /** @brief The sky of the scene (if any). */
    std::optional<Sky> sky;

    /**
     * @brief Collects information about a scene suitable for path tracing.
     *
     * @param[in] scene The scene to collect information from.
     * @return Information about the scene suitable for path tracing.
     */
    static constexpr SceneInfo from(const Scene& scene) {
        SceneInfo sceneInfo;
        sceneInfo.materials = scene.materials;
        sceneInfo.numOpaqueSpheres = 0;
        for (const Sphere& sphere : scene.spheres) {
            const Material& material = sceneInfo.materials[sphere.materialIdx];
            if (material.is_opaque()) {
                sceneInfo.spheres.push_back(sphere);
                sceneInfo.numOpaqueSpheres++;
            }
        }
        for (const Sphere& sphere : scene.spheres) {
            const Material& material = sceneInfo.materials[sphere.materialIdx];
            if (material.is_transparent()) {
                sceneInfo.spheres.push_back(sphere);
            }
        }
        for (auto [idx, sphere] : std::views::enumerate(sceneInfo.spheres)) {
            const Material& material = sceneInfo.materials[sphere.materialIdx];
            if (material.is_emissive()) {
                sceneInfo.emissiveSpheresIndices.push_back(idx);
            }
        }
        sceneInfo.lights = scene.lights;
        sceneInfo.sky = scene.sky;
        if (sceneInfo.sky) {
            assert(sceneInfo.sky->sunLight.is_directional());
            sceneInfo.lights.push_back(sceneInfo.sky->sunLight);
        }
        return sceneInfo;
    }

    /**
     * @brief Returns a view of the opaque spheres in the scene.
     *
     * @return A view of the opaque spheres in the scene.
     */
    constexpr auto opaque_spheres() const {
        return std::views::take(spheres, numOpaqueSpheres);
    }

    /**
     * @brief Returns a view of the transparent spheres in the scene.
     *
     * @return A view of the transparent spheres in the scene.
     */
    constexpr auto transparent_spheres() const {
        return std::views::drop(spheres, numOpaqueSpheres);
    }

};

/** @brief A ray. */
struct Ray {

    /** @brief The origin in world space. */
    Vec3<f32> origin;

    /** @brief The direction in world space. */
    Vec3<f32> direction;

    /** @brief The maximum distance for intersection tests. */
    f32 maxDistance;

};

/** @brief An intersection between a ray and a sphere. */
struct Hit {

    /** @brief The distance along the ray to the hit. */
    f32 distance;

    /** @brief The position of the hit. */
    Vec3<f32> position;

    /** @brief The surface normal at the hit. */
    Vec3<f32> normal;

    /** @brief The index of the sphere that was hit. */
    isize idx;

    /** @brief Whether the ray hit the sphere from outside. */
    bool entering;

};

/** @brief A contribution of a light for direct lighting. */
struct LightSample {

    /** @brief The direction towards the light. */
    Vec3<f32> direction;

    /** @brief The distance up to which the light must be visible. */
    f32 distance;

    /** @brief The incoming radiance. */
    Color radiance;

    /**
     * @brief The solid-angle PDF with which the direction was sampled, or
     * `std::nullopt` for delta bounces (which cannot be MIS-weighted).
     */
    std::optional<f32> pdf;

};

/** @brief The maximum number of bounces per path. */
static constexpr isize MAX_DEPTH = 16;

/** @brief The bounce after which russian roulette starts. */
static constexpr isize RUSSIAN_ROULETTE_DEPTH = 2;

/** @brief The minimum survival probability of russian roulette. */
static constexpr f32 MIN_SURVIVAL = 0.05F;

/** @brief The maximum number of surfaces a shadow ray may cross. */
static constexpr isize MAX_SHADOW_CROSSINGS = 16;

/** @brief The maximum indirect radiance contribution. */
static constexpr f32 MAX_INDIRECT_RADIANCE = 10.0F;

/**
 * @brief Returns the MIS weight for the first of two strategies using the power
 * heuristic.
 *
 * @param[in] lhs The PDF of the first strategy.
 * @param[in] rhs The PDF of the second strategy.
 * @return The MIS weight for the first strategy.
 */
static constexpr f32 power_heuristic(f32 lhs, f32 rhs) noexcept {
    return lhs * lhs / (lhs * lhs + rhs * rhs);
}

/**
 * @brief Tries to intersect a ray with a sphere.
 *
 * @param[in] sphere The sphere to test for intersection.
 * @param[in] ray    The ray to test for intersection.
 * @return The distance to the intersection, or `std::nullopt` if no
 * intersection exists.
 */
static constexpr std::optional<f32> intersect_sphere(
    const Sphere& sphere,
    const Ray& ray
) noexcept {
    Vec3<f32> f = ray.origin - sphere.center;
    f32 b = -f.dot(ray.direction);
    Vec3<f32> l = f + ray.direction * b;
    f32 d = sphere.radius * sphere.radius - l.norm_squared();
    if (d < 0.0F) {
        return std::nullopt;
    }
    f32 c = f.norm_squared() - sphere.radius * sphere.radius;
    f32 q = b + std::copysign(std::sqrt(d), b);
    f32 t0 = (q != 0.0F) ? c / q : 0.0F;
    f32 t1 = q;
    if (t0 > t1) {
        std::swap(t0, t1);
    }
    if ((t0 > 0.0F) && (t0 < ray.maxDistance)) {
        return t0;
    }
    if ((t1 > 0.0F) && (t1 < ray.maxDistance)) {
        return t1;
    }
    return std::nullopt;
}

/**
 * @brief Constructs a hit for a ray-sphere intersection.
 *
 * @param[in] ray      The ray that intersected the sphere.
 * @param[in] sphere   The sphere that was intersected.
 * @param[in] idx      The index of the sphere that was intersected.
 * @param[in] distance The distance along the ray to the hit.
 * @return A hit representing the intersection.
 */
static constexpr Hit make_hit(
    const Ray& ray,
    const Sphere& sphere,
    isize idx,
    f32 distance
) noexcept {
    Vec3<f32> position = ray.origin + ray.direction * distance;
    Vec3<f32> outwards = (position - sphere.center) * (1.0F / sphere.radius);
    position = sphere.center + outwards * sphere.radius;
    bool entering = ray.direction.dot(outwards) < 0.0F;
    return {
        .distance = distance,
        .position = position,
        .normal = entering ? outwards : -outwards,
        .idx = idx,
        .entering = entering
    };
}

/**
 * @brief Tries to intersect a ray with a set of spheres.
 *
 * @tparam Spheres The type of the container holding the spheres.
 * @param[in] spheres The set of spheres to test for intersection.
 * @param[in] ray     The ray to test for intersection.
 * @param[in] offset  The offset to add to the index of the intersected sphere.
 * @return The intersection between the ray and the closest sphere, or
 * `std::nullopt` if no intersection exists.
 */
static constexpr std::optional<Hit> intersect_spheres(
    std::span<const Sphere> spheres,
    Ray ray,
    isize offset = 0
) noexcept {
    isize closestIdx = -1;
    for (auto [idx, sphere] : std::views::enumerate(spheres)) {
        if (std::optional<f32> distance = intersect_sphere(sphere, ray)) {
            closestIdx = idx;
            ray.maxDistance = *distance;
        }
    }
    if (closestIdx == -1) {
        return std::nullopt;
    }
    return make_hit(
        ray,
        spheres[closestIdx],
        closestIdx + offset,
        ray.maxDistance
    );
}

/**
 * @brief Returns the radiance of a sky in a direction.
 *
 * @param[in] sky        The sky to sample.
 * @param[in] direction  The direction in which to sample.
 * @param[in] includeSun Whether to include the sun light's contribution.
 * @return The radiance of the sky in the direction.
 */
static constexpr Color sky_radiance(
    const Sky& sky,
    const Vec3<f32>& direction,
    bool includeSun
) noexcept {
    const Light::Directional& sun = *sky.sunLight.as_directional();
    f32 glow = std::max(direction.dot(-sun.direction), 0.0F);
    glow *= glow;
    glow *= glow;
    Color horizon = Color::lerp(sky.horizon, sky.sunLight.color(), glow);
    f32 t = std::clamp(direction.y, 0.0F, 1.0F);
    f32 inv = 1.0F - t;
    t = 1.0F - inv * inv * inv;
    Color radiance = Color::lerp(horizon, sky.zenith, t);
    if (
        includeSun
            && (sun.cosAngularRadius < 1.0F)
            && (direction.dot(-sun.direction) >= sun.cosAngularRadius)
    ) {
        f32 solidAngle = 2.0F * PI<f32> * (1.0F - sun.cosAngularRadius);
        radiance += sky.sunLight.color()
            * (sky.sunLight.intensity() / solidAngle);
    }
    return radiance;
}

/**
 * @brief Returns an origin for a new ray bouncing off a surface.
 *
 * @param[in] position  The position at which the surface was hit.
 * @param[in] normal    The surface normal at the hit (normalized).
 * @param[in] direction The direction the new ray is traveling towards.
 * @return The origin for the new ray.
 */
static constexpr Vec3<f32> offset_origin(
    const Vec3<f32>& position,
    Vec3<f32> normal,
    const Vec3<f32>& direction
) noexcept {
    constexpr f32 ORIGIN = 1.0F / 32.0F;
    constexpr f32 FLOAT_SCALE = 1.0F / 65536.0F;
    constexpr f32 INT_SCALE = 256.0F;
    auto nudge = [](f32 p, f32 n) {
        if (std::abs(p) < ORIGIN) {
            return p + FLOAT_SCALE * n;
        }
        i32 offset = static_cast<i32>(INT_SCALE * n);
        i32 bits = std::bit_cast<i32>(p);
        return std::bit_cast<f32>(bits + ((p < 0.0F) ? -offset : offset));
    };
    if (direction.dot(normal) < 0.0F) {
        normal = -normal;
    }
    return {
        .x = nudge(position.x, normal.x),
        .y = nudge(position.y, normal.y),
        .z = nudge(position.z, normal.z)
    };
}

/**
 * @brief Returns the fraction of light that survives a shadow ray.
 *
 * @param[in] sceneInfo The scene information.
 * @param[in] media     The media through which the shadow ray travels.
 * @param[in] ray       The shadow ray.
 * @return The fraction of light that survives the shadow ray.
 */
static constexpr Color shadow_transmittance(
    const SceneInfo& sceneInfo,
    MediumStack media,
    Ray ray
) noexcept {
    // Quickly test for intersections with opaque spheres, which fully block
    // the shadow ray, allowing us to skip more expensive calculations.
    for (const Sphere& sphere : sceneInfo.opaque_spheres()) {
        if (intersect_sphere(sphere, ray)) {
            return Color::splat(0.0F);
        }
    }
    Color transmittance = Color::splat(1.0F);
    for (isize i = 0; i < MAX_SHADOW_CROSSINGS; i++) {
        std::optional<Hit> hit = intersect_spheres(
            sceneInfo.transparent_spheres(),
            ray,
            sceneInfo.numOpaqueSpheres
        );
        if (!hit) {
            return transmittance * media.transmittance(ray.maxDistance);
        }
        const auto& [distance, position, normal, idx, entering] = *hit;
        const Sphere& sphere = sceneInfo.spheres[idx];
        const Material& material = sceneInfo.materials[sphere.materialIdx];
        transmittance *= media.transmittance(distance);
        auto transparent = material.as_transparent();
        DielectricBsdf bsdf(
            normal,
            -ray.direction,
            media.eta(idx, *transparent, entering)
        );
        transmittance *= transparent->transparency
            * bsdf.straight_transmittance();
        media.transition(idx, *transparent, entering);
        ray.origin = offset_origin(position, normal, ray.direction);
        ray.maxDistance -= distance;
    }
    return Color::splat(0.0F);
}

/**
 * @brief Samples a direction towards a sphere, uniformly by solid angle.
 *
 * @param[in]      center   The center of the sphere.
 * @param[in]      radius   The radius of the sphere.
 * @param[in]      position The position from which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return A sample with the direction, the distance to the near side of the
 * sphere, and the solid-angle PDF, or `std::nullopt` if the position is inside
 * the sphere.
 */
static constexpr std::optional<LightSample> sample_sphere(
    const Vec3<f32>& center,
    f32 radius,
    const Vec3<f32>& position,
    Xoshiro128StarStar& prng
) noexcept {
    Vec3<f32> axis = center - position;
    f32 distanceSquared = axis.norm_squared();
    f32 radiusSquared = radius * radius;
    if (distanceSquared <= radiusSquared) {
        return std::nullopt;
    }
    f32 distance = std::sqrt(distanceSquared);
    f32 versine = sphere_versine(distanceSquared, radius);
    Vec3<f32> direction = sample_cone(axis * (1.0F / distance), versine, prng);
    f32 b = direction.dot(axis);
    f32 d = b * b - (distanceSquared - radiusSquared);
    return LightSample {
        .direction = direction,
        .distance = b - std::sqrt(std::max(d, 0.0F)),
        .radiance = Color::splat(1.0F),
        .pdf = cone_pdf(versine)
    };
}

/**
 * @brief Samples a spherical light.
 *
 * @param[in]      light       The position of the light.
 * @param[in]      radius      The radius of the light.
 * @param[in]      attenuation The attenuation factors of the light.
 * @param[in]      position    The position at which to sample.
 * @param[in, out] prng        A pseudorandom number generator.
 * @return The sample of the spherical light.
 */
static constexpr LightSample sample_spherical_light(
    const Vec3<f32>& light,
    f32 radius,
    const Light::Attenuation& attenuation,
    const Vec3<f32>& position,
    Xoshiro128StarStar& prng
) noexcept {
    Vec3<f32> axis = light - position;
    f32 distance = axis.norm();
    LightSample result = {
        .direction = axis * (1.0F / distance),
        .distance = distance,
        .radiance = Color::splat(1.0F),
        .pdf = std::nullopt
    };
    if (radius > 0.0F) {
        if (auto sample = sample_sphere(light, radius, position, prng)) {
            result.direction = sample->direction;
            result.distance = sample->distance;
        }
    }
    const auto& [c, l, q] = attenuation;
    result.radiance *= 1.0F / (c + l * distance + q * distance * distance);
    return result;
}

/**
 * @brief Samples a point light.
 *
 * @param[in]      point    The properties of the point light.
 * @param[in]      position The position at which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The sample of the point light.
 */
static constexpr LightSample sample_light(
    const Light::Point& point,
    const Vec3<f32>& position,
    Xoshiro128StarStar& prng
) noexcept {
    return sample_spherical_light(
        point.position,
        point.radius,
        point.attenuation,
        position,
        prng
    );
}

/**
 * @brief Samples a spot light.
 *
 * @param[in]      spot     The properties of the spot light.
 * @param[in]      position The position at which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The sample of the spot light.
 */
static constexpr LightSample sample_light(
    const Light::Spot& spot,
    const Vec3<f32>& position,
    Xoshiro128StarStar& prng
) noexcept {
    LightSample sample = sample_spherical_light(
        spot.position,
        spot.radius,
        spot.attenuation,
        position,
        prng
    );
    f32 cosAngle = (position - spot.position).normalize().dot(spot.direction);
    f32 t = std::clamp(
        (cosAngle - spot.cosOuterCutoff)
            / std::max(spot.cosInnerCutoff - spot.cosOuterCutoff, 1.0E-6F),
        0.0F,
        1.0F
    );
    sample.radiance *= t * t * (3.0F - 2.0F * t);
    return sample;
}

/**
 * @brief Samples a directional light.
 *
 * @param[in]      directional The properties of the directional light.
 * @param[in]      position    The position at which to sample (unused).
 * @param[in, out] prng        A pseudorandom number generator.
 * @return The sample of the directional light.
 */
static constexpr LightSample sample_light(
    const Light::Directional& directional,
    [[maybe_unused]] const Vec3<f32>& position,
    Xoshiro128StarStar& prng
) noexcept {
    Vec3<f32> direction = -directional.direction;
    if (directional.cosAngularRadius < 1.0F) {
        direction = sample_cone(
            direction,
            1.0F - directional.cosAngularRadius,
            prng
        );
    }
    return {
        .direction = direction,
        .distance = INF<f32>,
        .radiance = Color::splat(1.0F),
        .pdf = std::nullopt
    };
}

/**
 * @brief Returns the contribution of a light sample.
 *
 * @param[in] sceneInfo The scene information.
 * @param[in] hit       The hit information.
 * @param[in] media     The media at the hit.
 * @param[in] bsdf      The BSDF at the hit.
 * @param[in] sample    The light sample.
 * @return The contribution of the light sample.
 */
static constexpr Color shade(
    const SceneInfo& sceneInfo,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    const LightSample& sample
) noexcept {
    f32 cosTheta = hit.normal.dot(sample.direction);
    if ((cosTheta <= 0.0F) || (sample.radiance.max() <= 0.0F)) {
        return Color::splat(0.0F);
    }
    f32 scale = cosTheta;
    if (sample.pdf) {
        scale *= power_heuristic(*sample.pdf, bsdf.pdf(sample.direction))
            / *sample.pdf;
    }
    Color contribution = sample.radiance * bsdf.eval(sample.direction) * scale;
    if (contribution.max() <= 0.0F) {
        return Color::splat(0.0F);
    }
    Ray shadowRay = {
        .origin = offset_origin(hit.position, hit.normal, sample.direction),
        .direction = sample.direction,
        .maxDistance = sample.distance
    };
    return contribution * shadow_transmittance(sceneInfo, media, shadowRay);
}

/**
 * @brief Returns the probability of picking a light.
 *
 * @param[in] sceneInfo The scene information.
 * @return The probability of picking a light.
 */
static constexpr f32 light_pick_pdf(const SceneInfo& sceneInfo) noexcept {
    isize total = std::ssize(sceneInfo.lights)
        + std::ssize(sceneInfo.emissiveSpheresIndices);
    return (total > 0) ? 1.0F / static_cast<f32>(total) : 0.0F;
}

/**
 * @brief Returns the direct lighting at a position.
 *
 * @param[in]      sceneInfo The scene information.
 * @param[in]      hit       The hit information.
 * @param[in]      media     The media at the hit.
 * @param[in]      bsdf      The BSDF at the hit.
 * @param[in, out] prng      A pseudorandom number generator.
 * @return The direct lighting at the position.
 */
static constexpr Color direct_lighting(
    const SceneInfo& sceneInfo,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    Xoshiro128StarStar& prng
) noexcept {
    isize lights = std::ssize(sceneInfo.lights);
    isize totalLights = lights + std::ssize(sceneInfo.emissiveSpheresIndices);
    if (totalLights == 0) {
        return Color::splat(0.0F);
    }
    f32 pickPdf = light_pick_pdf(sceneInfo);
    isize pick = std::min(
        static_cast<isize>(prng.next_f32() * static_cast<f32>(totalLights)),
        totalLights - 1
    );
    if (pick < lights) {
        const Light& light = sceneInfo.lights[pick];
        LightSample sample = std::visit(
            [&](const auto& kind) {
                return sample_light(kind, hit.position, prng);
            },
            light.kind()
        );
        sample.radiance *= light.color() * light.intensity();
        return shade(sceneInfo, hit, media, bsdf, sample) * (1.0F / pickPdf);
    }
    isize idx = sceneInfo.emissiveSpheresIndices[pick - lights];
    if (idx == hit.idx) {
        return Color::splat(0.0F);
    }
    const Sphere& sphere = sceneInfo.spheres[idx];
    std::optional<LightSample> sample = sample_sphere(
        sphere.center,
        sphere.radius,
        hit.position,
        prng
    );
    if (!sample) {
        return Color::splat(0.0F);
    }
    sample->distance *= 0.999F;
    sample->radiance = sceneInfo.materials[sphere.materialIdx].emission();
    if (sample->pdf) {
        *sample->pdf *= pickPdf;
    }
    return shade(sceneInfo, hit, media, bsdf, *sample);
}

/**
 * @brief Restricts the radiance based on the maximum indirect contribution and
 * depth.
 *
 * @param[in] radiance The radiance that may need to be restricted.
 * @param[in] depth    The trace depth.
 * @return The restricted radiance.
 *
 * @note This is biased, but a reliable method for reducing bright pixel
 * ("fireflies") caused by high indirect radiance contributions. For direct
 * contributions (i.e., `depth == 0`), this restriction is not applied.
 */
static constexpr Color restrict_radiance(
    const Color& radiance,
    isize depth
) noexcept {
    if (depth == 0) {
        return radiance;
    }
    f32 max = radiance.max();
    return (max > MAX_INDIRECT_RADIANCE)
        ? radiance * (MAX_INDIRECT_RADIANCE / max)
        : radiance;
}

/**
 * @brief Traces a ray through a scene, accumulating radiance along the way.
 *
 * @param[in]      sceneInfo The scene information.
 * @param[in]      media     The media at the ray origin.
 * @param[in]      ray       The ray to trace.
 * @param[in, out] prng      A pseudorandom number generator.
 * @return The accumulated radiance along the ray.
 */
static constexpr Color trace(
    const SceneInfo& sceneInfo,
    MediumStack media,
    Ray ray,
    Xoshiro128StarStar& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    Color throughput = Color::splat(1.0F);
    std::optional<f32> previousPdf;
    for (isize depth = 0; depth < MAX_DEPTH; depth++) {
        std::optional<Hit> hit = intersect_spheres(sceneInfo.spheres, ray);
        if (!hit) {
            if (sceneInfo.sky) {
                radiance += restrict_radiance(
                    throughput
                        * sky_radiance(
                            *sceneInfo.sky,
                            ray.direction,
                            !previousPdf
                        ),
                    depth
                );
            }
            break;
        }
        const auto& [distance, position, normal, idx, entering] = *hit;
        const Sphere& sphere = sceneInfo.spheres[idx];
        const Material& material = sceneInfo.materials[sphere.materialIdx];
        throughput *= media.transmittance(distance);
        if (entering && material.is_emissive()) {
            f32 weight = 1.0F;
            if (previousPdf) {
                weight = power_heuristic(
                    *previousPdf,
                    sphere_cone_pdf(sphere.center, sphere.radius, ray.origin)
                        * light_pick_pdf(sceneInfo)
                );
            }
            radiance += restrict_radiance(
                throughput * material.emission() * weight,
                depth
            );
        }
        Bounce bounce;
        Vec3<f32> view = -ray.direction;
        auto transparent = material.as_transparent();
        if (transparent && (prng.next_f32() < transparent->transparency)) {
            DielectricBsdf bsdf(
                normal,
                view,
                media.eta(idx, *transparent, entering)
            );
            bounce = bsdf.sample(prng);
        }
        else {
            MicrofacetBsdf bsdf(
                material.f0(),
                material.diffuse(),
                material.roughness(),
                normal,
                view
            );
            radiance += restrict_radiance(
                throughput
                    * direct_lighting(sceneInfo, *hit, media, bsdf, prng),
                depth
            );
            std::optional<Bounce> temp = bsdf.sample(prng);
            if (!temp) {
                break;
            }
            bounce = *temp;
        }
        throughput *= bounce.weight;
        f32 throughputMax = throughput.max();
        if (throughputMax <= 0.0F) {
            break;
        }
        if (depth >= RUSSIAN_ROULETTE_DEPTH) {
            f32 survival = std::clamp(throughputMax, MIN_SURVIVAL, 1.0F);
            if (prng.next_f32() >= survival) {
                break;
            }
            throughput *= 1.0F / survival;
        }
        if (bounce.transmitted) {
            media.transition(idx, *transparent, entering);
        }
        previousPdf = bounce.pdf;
        ray.origin = offset_origin(position, normal, bounce.direction);
        ray.direction = bounce.direction;
    }
    return radiance;
}

CGFS_API void pathtrace(
    const Scene& scene,
    ThreadPool& threadPool,
    Film& film
) {
    SceneInfo sceneInfo = SceneInfo::from(scene);
    f32 width = static_cast<f32>(film.width());
    f32 height = static_cast<f32>(film.height());
    f32 minX = -width / 2.0F;
    f32 maxY = height / 2.0F;
    f32 scale = 2.0F * std::tan(scene.camera.fov() / 2.0F) / height;
    Mat3<f32> viewToWorld = scene.camera.orientation().to_mat3();
    Vec3<f32> position = scene.camera.position();
    MediumStack media = MediumStack::initial(
        sceneInfo.spheres,
        sceneInfo.materials,
        position
    );
    std::random_device device;
    u64 baseSeed = (static_cast<u64>(device()) << 32) | device();
    for (isize y = 0; y < film.height(); y++) {
        SplitMix64 splitMix64(baseSeed + y);
        u64 seed = splitMix64.next();
        threadPool.post(
            [&, y, seed]() {
                Xoshiro128StarStar prng(seed);
                for (isize x = 0; x < film.width(); x++) {
                    Vec3<f32> direction = {
                        .x = (minX + static_cast<f32>(x) + prng.next_f32())
                            * scale,
                        .y = (maxY - static_cast<f32>(y + 1) + prng.next_f32())
                            * scale,
                        .z = -1.0F
                    };
                    Ray ray = {
                        .origin = position,
                        .direction = (viewToWorld * direction).normalize(),
                        .maxDistance = INF<f32>
                    };
                    Color color = trace(sceneInfo, media, ray, prng);
                    film.add_sample(x, y, color);
                }
            }
        );
    }
    threadPool.wait_idle();
    film.end_frame();
}

}