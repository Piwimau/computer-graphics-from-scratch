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
#include "cgfs/prng.hpp"
#include "cgfs/sampling.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"
#include "dielectric-bsdf.hpp"
#include "medium-stack.hpp"
#include "microfacet-bsdf.hpp"

namespace cgfs {

/** @brief Information about a scene required for path tracing. */
struct SceneInfo {

    /** @brief The number of opaque objects in the scene. */
    isize numOpaqueObjects;

    /** @brief The indices of the emissive objects in the scene. */
    std::vector<isize> emissiveObjectIndices;

    /**
     * @brief Collects information about a scene required for path tracing.
     *
     * @param[in] objects   The objects in the scene.
     * @param[in] materials The materials of the objects in the scene.
     * @return Information about the scene required for path tracing.
     */
    static constexpr SceneInfo from(
        std::span<const Object> objects,
        std::span<const Material> materials
    ) {
        SceneInfo sceneInfo = { };
        for (auto [idx, object] : std::views::enumerate(objects)) {
            const Material& material = materials[object.material_idx()];
            if (material.is_opaque()) {
                sceneInfo.numOpaqueObjects++;
            }
            if (material.is_emissive()) {
                sceneInfo.emissiveObjectIndices.push_back(idx);
            }
        }
        return sceneInfo;
    }

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
 * @brief Tries to intersect a ray with a set of objects.
 *
 * @param[in] objects The set of objects to test for intersection.
 * @param[in] ray     The ray to test for intersection.
 * @param[in] offset  The offset to add to the index of the intersected object.
 * @return The intersection between the ray and the closest object, or
 * `std::nullopt` if no intersection exists.
 */
static constexpr std::optional<Hit> intersect_objects(
    std::span<const Object> objects,
    Ray ray,
    isize offset = 0
) noexcept {
    std::optional<Hit> closestHit;
    for (auto [idx, object] : std::views::enumerate(objects)) {
        if (std::optional<Hit> hit = object.intersect(ray)) {
            closestHit = hit;
            closestHit->idx = idx + offset;
            ray.maxDistance = closestHit->distance;
        }
    }
    return closestHit;
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
            && sun.cosAngularRadius < 1.0F
            && direction.dot(-sun.direction) >= sun.cosAngularRadius
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
 * @param[in] objects   The objects in the scene.
 * @param[in] materials The materials of the objects in the scene.
 * @param[in] sceneInfo The scene information.
 * @param[in] media     The media through which the shadow ray travels.
 * @param[in] ray       The shadow ray.
 * @return The fraction of light that survives the shadow ray.
 */
static constexpr Color shadow_transmittance(
    std::span<const Object> objects,
    std::span<const Material> materials,
    const SceneInfo& sceneInfo,
    MediumStack media,
    Ray ray
) noexcept {
    // Quickly test for intersections with opaque objects, which fully block
    // the shadow ray, allowing us to skip more expensive calculations.
    auto opaqueObjects = std::views::take(objects, sceneInfo.numOpaqueObjects);
    for (const auto& object : opaqueObjects) {
        if (object.intersect(ray)) {
            return Color::splat(0.0F);
        }
    }
    Color transmittance = Color::splat(1.0F);
    auto transparentObjects = std::views::drop(
        objects,
        sceneInfo.numOpaqueObjects
    );
    for (isize i = 0; i < MAX_SHADOW_CROSSINGS; i++) {
        std::optional<Hit> hit = intersect_objects(
            transparentObjects,
            ray,
            sceneInfo.numOpaqueObjects
        );
        if (!hit) {
            return transmittance * media.transmittance(ray.maxDistance);
        }
        const auto& [distance, position, normal, idx, entering] = *hit;
        const Material& material = materials[objects[idx].material_idx()];
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
 * @param[in] objects   The objects in the scene.
 * @param[in] materials The materials of the objects in the scene.
 * @param[in] sceneInfo The scene information.
 * @param[in] hit       The hit information.
 * @param[in] media     The media at the hit.
 * @param[in] bsdf      The BSDF at the hit.
 * @param[in] sample    The light sample.
 * @return The contribution of the light sample.
 */
static constexpr Color shade(
    std::span<const Object> objects,
    std::span<const Material> materials,
    const SceneInfo& sceneInfo,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    const LightSample& sample
) noexcept {
    f32 cosTheta = hit.normal.dot(sample.direction);
    if (cosTheta <= 0.0F || sample.radiance.max() <= 0.0F) {
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
    return contribution
        * shadow_transmittance(objects, materials, sceneInfo, media, shadowRay);
}

/**
 * @brief Returns the direct lighting at a position.
 *
 * @param[in]      scene     The scene containing objects, materials and lights.
 * @param[in]      sceneInfo The scene information.
 * @param[in]      hit       The hit information.
 * @param[in]      media     The media at the hit.
 * @param[in]      bsdf      The BSDF at the hit.
 * @param[in, out] prng      A pseudorandom number generator.
 * @return The direct lighting at the position.
 */
static constexpr Color direct_lighting(
    const Scene& scene,
    const SceneInfo& sceneInfo,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    Xoshiro128StarStar& prng
) noexcept {
    isize lights = std::ssize(scene.lights);
    isize emissiveObjects = std::ssize(sceneInfo.emissiveObjectIndices);
    isize totalLights = lights + emissiveObjects + (scene.sky ? 1 : 0);
    if (totalLights == 0) {
        return Color::splat(0.0F);
    }
    f32 pickPdf = 1.0F / static_cast<f32>(totalLights);
    isize pick = std::min(
        static_cast<isize>(prng.next_f32() * static_cast<f32>(totalLights)),
        totalLights - 1
    );
    if (pick < lights || (scene.sky && pick == totalLights - 1)) {
        const Light& light = (pick < lights)
            ? scene.lights[pick]
            : scene.sky->sunLight;
        LightSample sample = std::visit(
            [&](const auto& kind) {
                return sample_light(kind, hit.position, prng);
            },
            light.kind()
        );
        sample.radiance *= light.color() * light.intensity();
        return shade(
            scene.objects,
            scene.materials,
            sceneInfo,
            hit,
            media,
            bsdf,
            sample
        ) * (1.0F / pickPdf);
    }
    isize idx = sceneInfo.emissiveObjectIndices[pick - lights];
    if (idx == hit.idx) {
        return Color::splat(0.0F);
    }
    const auto& object = scene.objects[idx];
    if (auto sample = object.sample_point(hit.position, prng)) {
        LightSample lightSample = {
            .direction = sample->direction,
            .distance = sample->distance * 0.999F,
            .radiance = scene.materials[object.material_idx()].emission(),
            .pdf = sample->pdf
                ? std::optional(*sample->pdf * pickPdf)
                : std::nullopt
        };
        return shade(
            scene.objects,
            scene.materials,
            sceneInfo,
            hit,
            media,
            bsdf,
            lightSample
        );
    }
    return Color::splat(0.0F);
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
 * @param[in]      scene     The scene to trace the ray through.
 * @param[in]      sceneInfo The scene information.
 * @param[in]      media     The media at the ray origin.
 * @param[in]      ray       The ray to trace.
 * @param[in, out] prng      A pseudorandom number generator.
 * @return The accumulated radiance along the ray.
 */
static constexpr Color trace(
    const Scene& scene,
    const SceneInfo& sceneInfo,
    MediumStack media,
    Ray ray,
    Xoshiro128StarStar& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    Color throughput = Color::splat(1.0F);
    std::optional<f32> previousPdf;
    for (isize depth = 0; depth < MAX_DEPTH; depth++) {
        std::optional<Hit> hit = intersect_objects(scene.objects, ray);
        if (!hit) {
            if (scene.sky) {
                radiance += restrict_radiance(
                    throughput
                        * sky_radiance(*scene.sky, ray.direction, !previousPdf),
                    depth
                );
            }
            break;
        }
        const auto& [distance, position, normal, idx, entering] = *hit;
        const auto& object = scene.objects[idx];
        const Material& material = scene.materials[object.material_idx()];
        throughput *= media.transmittance(distance);
        if (entering && material.is_emissive()) {
            f32 weight = 1.0F;
            if (previousPdf) {
                if (auto objectPdf = object.sample_pdf(ray.origin)) {
                    isize totalLights = std::ssize(scene.lights)
                        + std::ssize(sceneInfo.emissiveObjectIndices)
                        + (scene.sky ? 1 : 0);
                    f32 lightPickPdf = (totalLights > 0)
                        ? 1.0F / static_cast<f32>(totalLights)
                        : 0.0F;
                    weight = power_heuristic(
                        *previousPdf,
                        *objectPdf * lightPickPdf
                    );
                }
            }
            radiance += restrict_radiance(
                throughput * material.emission() * weight,
                depth
            );
        }
        Bounce bounce;
        Vec3<f32> view = -ray.direction;
        auto transparent = material.as_transparent();
        if (transparent && prng.next_f32() < transparent->transparency) {
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
                    * direct_lighting(
                        scene,
                        sceneInfo,
                        *hit,
                        media,
                        bsdf,
                        prng
                    ),
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
    SceneInfo sceneInfo = SceneInfo::from(scene.objects, scene.materials);
    f32 width = static_cast<f32>(film.width());
    f32 height = static_cast<f32>(film.height());
    f32 minX = -width / 2.0F;
    f32 maxY = height / 2.0F;
    f32 scale = 2.0F * std::tan(scene.camera.fov() / 2.0F) / height;
    Mat3<f32> viewToWorld = scene.camera.orientation().to_mat3();
    Vec3<f32> position = scene.camera.position();
    MediumStack media = MediumStack::initial(
        scene.objects,
        scene.materials,
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
                    Color color = trace(scene, sceneInfo, media, ray, prng);
                    film.add_sample(x, y, color);
                }
            }
        );
    }
    threadPool.wait_idle();
    film.end_frame();
}

}