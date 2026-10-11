#include <algorithm>
#include <bit>
#include <cmath>
#include <optional>
#include <random>
#include "cgfs/color.hpp"
#include "cgfs/pathtracer.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"
#include "cone.hpp"
#include "dielectric-bsdf.hpp"
#include "medium-stack.hpp"
#include "microfacet-bsdf.hpp"
#include "prng.hpp"

namespace cgfs {

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

    /** @brief The sphere that was hit. */
    const Sphere& sphere;

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
static constexpr isize MAX_DEPTH = 32;

/** @brief The bounce after which russian roulette starts. */
static constexpr isize RUSSIAN_ROULETTE_DEPTH = 3;

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
 * @brief Tries to intersect a ray with the scene.
 *
 * @param[in] scene The scene to test for intersection.
 * @param[in] ray   The ray to test for intersection.
 * @return The intersection between the ray and the closest sphere, or
 * `std::nullopt` if no intersection exists.
 */
static constexpr std::optional<Hit> intersect_scene(
    const Scene& scene,
    Ray ray
) noexcept {
    const Sphere* closest = nullptr;
    for (const Sphere& sphere : scene.spheres) {
        if (std::optional<f32> t = intersect_sphere(sphere, ray)) {
            closest = &sphere;
            ray.maxDistance = *t;
        }
    }
    if (!closest) {
        return std::nullopt;
    }
    Vec3<f32> position = ray.origin + ray.direction * ray.maxDistance;
    Vec3<f32> outwards = (position - closest->center).normalize();
    position = closest->center + outwards * closest->radius;
    bool entering = ray.direction.dot(outwards) < 0.0F;
    return Hit {
        .distance = ray.maxDistance,
        .position = position,
        .normal = entering ? outwards : -outwards,
        .sphere = *closest,
        .entering = entering
    };
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
 * @param[in] scene The scene to test for shadow ray intersections.
 * @param[in] media The media through which the shadow ray travels.
 * @param[in] ray   The shadow ray.
 * @return The fraction of light that survives the shadow ray.
 */
static constexpr Color shadow_transmittance(
    const Scene& scene,
    MediumStack media,
    Ray ray
) noexcept {
    Color transmittance = Color::splat(1.0F);
    for (isize i = 0; i < MAX_SHADOW_CROSSINGS; i++) {
        std::optional<Hit> hit = intersect_scene(scene, ray);
        if (!hit) {
            return transmittance * media.transmittance(ray.maxDistance);
        }
        const auto& [distance, position, normal, sphere, entering] = *hit;
        if (!sphere.material.is_transparent()) {
            return Color::splat(0.0F);
        }
        transmittance *= media.transmittance(distance);
        auto transparent = sphere.material.as_transparent();
        auto [etaI, etaT] = media.iors(sphere, entering);
        DielectricBsdf bsdf(normal, -ray.direction, etaI / etaT);
        transmittance *= transparent->transparency
            * bsdf.straight_transmittance();
        media.transition(sphere, entering);
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
    Prng& prng
) noexcept {
    Vec3<f32> axis = center - position;
    if (axis.norm_squared() <= radius * radius) {
        return std::nullopt;
    }
    Cone cone = Cone::from(center, radius, position);
    Vec3<f32> direction = cone.sample(prng);
    f32 distance = axis.norm();
    f32 b = distance * direction.dot(cone.axis);
    f32 d = b * b - (distance * distance - radius * radius);
    return LightSample {
        .direction = direction,
        .distance = b - std::sqrt(std::max(d, 0.0F)),
        .radiance = Color::splat(1.0F),
        .pdf = cone.pdf()
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
    Prng& prng
) noexcept {
    Vec3<f32> axis = light - position;
    f32 distance = axis.norm();
    LightSample result = {
        .direction = axis / distance,
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
    result.radiance /= c + l * distance + q * distance * distance;
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
    Prng& prng
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
    Prng& prng
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
    Prng& prng
) noexcept {
    Vec3<f32> direction = -directional.direction;
    if (directional.cosAngularRadius < 1.0F) {
        Cone cone = Cone::from(direction, directional.cosAngularRadius);
        direction = cone.sample(prng);
    }
    return {
        .direction = direction,
        .distance = INF<f32>,
        .radiance = Color::splat(1.0F),
        .pdf = std::nullopt
    };
}

/**
 * @brief Samples a light.
 *
 * @param[in]      light    The light to sample.
 * @param[in]      position The position at which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The sample of the light.
 */
static constexpr LightSample sample_light(
    const Light& light,
    const Vec3<f32>& position,
    Prng& prng
) noexcept {
    LightSample sample = std::visit(
        [&](const auto& kind) { return sample_light(kind, position, prng); },
        light.kind()
    );
    sample.radiance *= light.color() * light.intensity();
    return sample;
}

/**
 * @brief Samples an emissive sphere.
 *
 * @param[in]      sphere   The emissive sphere.
 * @param[in]      position The position at which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The sample of the emissive sphere, or `std::nullopt` if the
 * position is inside the sphere.
 */
static constexpr std::optional<LightSample> sample_emissive_sphere(
    const Sphere& sphere,
    const Vec3<f32>& position,
    Prng& prng
) noexcept {
    std::optional<LightSample> sample = sample_sphere(
        sphere.center,
        sphere.radius,
        position,
        prng
    );
    if (sample) {
        sample->distance *= 0.999F;
        sample->radiance = sphere.material.emission();
    }
    return sample;
}

/**
 * @brief Returns the contribution of a light sample.
 *
 * @param[in] scene  The scene containing the lights and objects.
 * @param[in] hit    The hit information.
 * @param[in] media  The media at the hit.
 * @param[in] bsdf   The BSDF at the hit.
 * @param[in] sample The light sample.
 * @return The contribution of the light sample.
 */
static constexpr Color shade(
    const Scene& scene,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    const LightSample& sample
) noexcept {
    f32 cosTheta = hit.normal.dot(sample.direction);
    if (cosTheta <= 0.0F) {
        return Color::splat(0.0F);
    }
    Ray shadowRay = {
        .origin = offset_origin(hit.position, hit.normal, sample.direction),
        .direction = sample.direction,
        .maxDistance = sample.distance
    };
    Color visibility = shadow_transmittance(scene, media, shadowRay);
    if (visibility.max() <= 0.0F) {
        return Color::splat(0.0F);
    }
    f32 scale = cosTheta;
    if (sample.pdf) {
        scale = cosTheta
            * power_heuristic(*sample.pdf, bsdf.pdf(sample.direction))
            / *sample.pdf;
    }
    return sample.radiance * visibility * bsdf.eval(sample.direction) * scale;
}

/**
 * @brief Returns the direct lighting at a position.
 *
 * @param[in]      scene The scene containing the lights and objects.
 * @param[in]      hit   The hit information.
 * @param[in]      media The media at the hit.
 * @param[in]      bsdf  The BSDF at the hit.
 * @param[in, out] prng  A pseudorandom number generator.
 * @return The direct lighting at the position.
 */
static constexpr Color direct_lighting(
    const Scene& scene,
    const Hit& hit,
    const MediumStack& media,
    const MicrofacetBsdf& bsdf,
    Prng& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    for (const Light& light : scene.lights) {
        radiance += shade(
            scene,
            hit,
            media,
            bsdf,
            sample_light(light, hit.position, prng)
        );
    }
    for (const Sphere& sphere : scene.spheres) {
        if (!sphere.material.is_emissive() || (&sphere == &hit.sphere)) {
            continue;
        }
        if (auto sample = sample_emissive_sphere(sphere, hit.position, prng)) {
            radiance += shade(scene, hit, media, bsdf, *sample);
        }
    }
    return radiance;
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
    assert(depth >= 0);
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
 * @param[in]      scene The scene to trace the ray through.
 * @param[in]      media The media at the ray origin.
 * @param[in]      ray   The ray to trace.
 * @param[in, out] prng  A pseudorandom number generator.
 * @return The accumulated radiance along the ray.
 */
static constexpr Color trace(
    const Scene& scene,
    MediumStack media,
    Ray ray,
    Prng& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    Color throughput = Color::splat(1.0F);
    std::optional<f32> previousPdf;
    for (isize depth = 0; depth < MAX_DEPTH; depth++) {
        std::optional<Hit> hit = intersect_scene(scene, ray);
        if (!hit) {
            break;
        }
        const auto& [distance, position, normal, sphere, entering] = *hit;
        const Material& material = sphere.material;
        throughput *= media.transmittance(distance);
        if (entering && material.is_emissive()) {
            f32 weight = 1.0F;
            if (previousPdf) {
                Cone cone = Cone::from(
                    sphere.center,
                    sphere.radius,
                    ray.origin
                );
                weight = power_heuristic(*previousPdf, cone.pdf());
            }
            radiance += restrict_radiance(
                throughput * material.emission() * weight,
                depth
            );
        }
        std::optional<Bounce> bounce;
        Vec3<f32> view = -ray.direction;
        auto transparent = material.as_transparent();
        if (transparent && (prng.next_f32() < transparent->transparency)) {
            auto [etaI, etaT] = media.iors(sphere, entering);
            DielectricBsdf bsdf(normal, view, etaI / etaT);
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
                throughput * direct_lighting(scene, *hit, media, bsdf, prng),
                depth
            );
            bounce = bsdf.sample(prng);
        }
        if (!bounce) {
            break;
        }
        throughput *= bounce->weight;
        f32 throughputMax = throughput.max();
        if (throughputMax <= 0.0F) {
            break;
        }
        if (depth >= RUSSIAN_ROULETTE_DEPTH) {
            f32 survival = std::clamp(throughputMax, MIN_SURVIVAL, 1.0F);
            if (prng.next_f32() >= survival) {
                break;
            }
            throughput /= survival;
        }
        if (bounce->transmitted) {
            media.transition(sphere, entering);
        }
        previousPdf = bounce->pdf;
        ray.origin = offset_origin(position, normal, bounce->direction);
        ray.direction = bounce->direction;
    }
    return radiance;
}

void pathtrace(const Scene& scene, ThreadPool& threadPool, Film& film) {
    f32 width = static_cast<f32>(film.width());
    f32 height = static_cast<f32>(film.height());
    f32 minX = -width / 2.0F;
    f32 maxY = height / 2.0F;
    f32 scale = 2.0F * std::tan(scene.camera.fov() / 2.0F) / height;
    Mat3<f32> viewToWorld = scene.camera.orientation().to_mat3();
    Vec3<f32> position = scene.camera.position();
    MediumStack media = MediumStack::initial(scene.spheres, position);
    std::random_device device;
    for (isize y = 0; y < film.height(); y++) {
        u64 seed = (static_cast<u64>(device()) << 32) | device();
        threadPool.post(
            [&, y, seed]() {
                Prng prng(seed);
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
                    Color color = trace(scene, media, ray, prng);
                    film.add_sample(x, y, color);
                }
            }
        );
    }
    threadPool.wait_idle();
    film.end_frame();
}

}