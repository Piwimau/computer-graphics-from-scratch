#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <utility>
#include "cgfs/color.hpp"
#include "cgfs/overload.hpp"
#include "cgfs/pathtracer.hpp"
#include "cgfs/prng.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A ray. */
struct Ray {

    /** @brief The origin in world space. */
    Vec3<f32> origin;

    /** @brief The direction in world space. */
    Vec3<f32> direction;

    /** @brief The minimum distance for intersection tests. */
    f32 minDistance;

    /** @brief The maximum distance for intersection tests. */
    f32 maxDistance;

};

/** @brief An intersection between a ray and a surface. */
struct Hit {

    /** @brief The distance along the ray to the hit. */
    f32 distance;

    /** @brief The position of the hit. */
    Vec3<f32> position;

    /** @brief The surface normal at the hit. */
    Vec3<f32> normal;

    /** @brief The surface material at the hit. */
    const Material* material;

    /** @brief Whether the ray hit the surface from outside. */
    bool entering;

};

/** @brief A stack for tracking nested media of transparent objects. */
class MediumStack final {
private:

    /** @brief The maximum nesting depth. */
    static constexpr isize CAPACITY = 8;

    /** @brief The materials of the nested media. */
    std::array<const Material*, CAPACITY> _materials = { };

    /** @brief The current size of this stack. */
    isize _size = 0;

    /**
     * @brief Returns the index of a material, or `-1` if it is not found.
     *
     * @param[in] material The material to find.
     * @return The index of the material, or `-1` if it is not found.
     */
    constexpr isize find(const Material* material) const noexcept {
        for (isize i = _size - 1; i >= 0; i--) {
            if (_materials[i] == material) {
                return i;
            }
        }
        return -1;
    }

public:

    /**
     * @brief Determines whether this stack is empty.
     *
     * @return `true` if this stack is empty, otherwise `false`.
     */
    constexpr bool empty() const noexcept {
        return _size == 0;
    }

    /**
     * @brief Returns the index of refraction of the current medium.
     *
     * @return The index of refraction of the current medium.
     */
    constexpr f32 ior() const noexcept {
        return empty() ? 1.0F : _materials[_size - 1]->as_transparent()->ior;
    }

    /**
     * @brief Returns the absorption of the current medium.
     *
     * @return The absorption of the current medium.
     */
    constexpr Color absorption() const noexcept {
        return empty()
            ? Color::splat(0.0F)
            : _materials[_size - 1]->as_transparent()->absorption;
    }

    /**
     * @brief Returns the index of refraction of the medium surrounding a
     * material.
     *
     * @param[in] material The material to find.
     * @return The index of refraction of the medium surrounding the material.
     */
    constexpr f32 surrounding_ior(const Material* material) const noexcept {
        isize i = find(material);
        if (i < 0) {
            return ior();
        }
        return (i > 0) ? _materials[i - 1]->as_transparent()->ior : 1.0F;
    }

    /**
     * @brief Pushes a material onto this stack.
     *
     * @param[in] material The material to push.
     */
    constexpr void push(const Material* material) noexcept {
        if (_size < CAPACITY) {
            _materials[_size++] = material;
        }
    }

    /**
     * @brief Enters or leaves the object that was hit after a transmission.
     *
     * @param[in] hit The hit information of the surface being crossed.
     */
    constexpr void transition(const Hit& hit) noexcept {
        if (hit.entering) {
            push(hit.material);
            return;
        }
        isize i = find(hit.material);
        if (i < 0) {
            return;
        }
        for (isize j = i; j + 1 < _size; j++) {
            _materials[j] = _materials[j + 1];
        }
        _size--;
    }

};

/** @brief A contribution of a light for direct lighting. */
struct LightSample {

    /** @brief The direction towards the light. */
    Vec3<f32> direction;

    /** @brief The distance to the light. */
    f32 distance;

    /** @brief The incoming radiance. */
    Color radiance;

};

/** @brief A continuation of a path at a surface. */
struct Bounce {

    /** @brief The outgoing direction. */
    Vec3<f32> direction;

    /** @brief The weight of the bounce. */
    Color weight;

    /** @brief Whether the bounce is a transmission. */
    bool transmitted = false;

};

/** @brief The value of pi. */
static constexpr f32 PI = std::numbers::pi_v<f32>;

/** @brief The value of infinity. */
static constexpr f32 INF = std::numeric_limits<f32>::infinity();

/** @brief The maximum number of bounces per path. */
static constexpr isize MAX_DEPTH = 32;

/** @brief The bounce after which russian roulette starts. */
static constexpr isize RUSSIAN_ROULETTE_DEPTH = 3;

/** @brief The minimum GGX alpha to avoid degenerate lobes. */
static constexpr f32 MIN_ALPHA = 0.001F;

/** @brief The maximum contribution of indirect bounces. */
static constexpr f32 MAX_INDIRECT = 10.0F;

/** @brief The maximum number of surfaces a shadow ray may cross. */
static constexpr isize MAX_SHADOW_CROSSINGS = 16;

/** @brief The color of the sky at the horizon. */
static constexpr Color SKY_HORIZON = { 0.6F, 0.7F, 0.8F };

/** @brief The color of the sky at the zenith. */
static constexpr Color SKY_ZENITH = { 0.1F, 0.2F, 0.6F };

/**
 * @brief Returns the initial stack of media containing a position.
 *
 * @param[in] scene    The scene containing the objects.
 * @param[in] position The position to check for containment.
 * @return The initial stack of media containing the position.
 */
static constexpr MediumStack initial_media(
    const Scene& scene,
    const Vec3<f32>& position
) {
    std::vector<const Sphere*> containing;
    for (const Sphere& sphere : scene.spheres) {
        Vec3<f32> offset = position - sphere.center;
        if (
            sphere.material.as_transparent()
                && (offset.dot(offset) < sphere.radius * sphere.radius)
        ) {
            containing.push_back(&sphere);
        }
    }
    std::ranges::sort(
        containing,
        [](const Sphere* a, const Sphere* b) { return a->radius > b->radius; }
    );
    MediumStack media;
    for (const Sphere* sphere : containing) {
        media.push(&sphere->material);
    }
    return media;
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
    f32 d = sphere.radius * sphere.radius - l.dot(l);
    if (d < 0.0F) {
        return std::nullopt;
    }
    f32 c = f.dot(f) - sphere.radius * sphere.radius;
    f32 q = b + std::copysign(std::sqrt(d), b);
    f32 t0 = (q != 0.0F) ? c / q : 0.0F;
    f32 t1 = q;
    if (t0 > t1) {
        std::swap(t0, t1);
    }
    if ((t0 > ray.minDistance) && (t0 < ray.maxDistance)) {
        return t0;
    }
    if ((t1 > ray.minDistance) && (t1 < ray.maxDistance)) {
        return t1;
    }
    return std::nullopt;
}

/**
 * @brief Tries to intersect a ray with a scene.
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
        if (auto t = intersect_sphere(sphere, ray)) {
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
        .material = &closest->material,
        .entering = entering
    };
}

/**
 * @brief Returns the color of the sky in a direction.
 *
 * @param[in] direction The direction in which to sample.
 * @return The color of the sky in the direction.
 */
static constexpr Color sky_color(const Vec3<f32>& direction) noexcept {
    f32 t = std::clamp(direction.y, 0.0F, 1.0F);
    f32 inverse = 1.0F - t;
    return Color::lerp(SKY_HORIZON, SKY_ZENITH, 1.0F - inverse * inverse);
}

/**
 * @brief Returns an orthonormal basis around a normal.
 *
 * @param[in] normal The normal around which to build the basis (normalized).
 * @return The orthonormal basis around the normal.
 */
static constexpr std::pair<Vec3<f32>, Vec3<f32>> make_orthonormal_basis(
    const Vec3<f32>& normal
) noexcept {
    f32 sign = std::copysign(1.0F, normal.z);
    f32 a = -1.0F / (sign + normal.z);
    f32 b = normal.x * normal.y * a;
    Vec3<f32> tangent = {
        .x = 1.0F + sign * normal.x * normal.x * a,
        .y = sign * b,
        .z = -sign * normal.x
    };
    Vec3<f32> bitangent = {
        .x = b,
        .y = sign + normal.y * normal.y * a,
        .z = -normal.y
    };
    return { tangent, bitangent };
}

/**
 * @brief Samples a cosine-weighted direction in a hemisphere around a normal.
 *
 * @param[in]      normal The normal around which to sample (normalized).
 * @param[in, out] prng   A pseudorandom number generator.
 * @return A cosine-weighted direction in the hemisphere around the normal.
 */
static constexpr Vec3<f32> sample_cosine_hemisphere(
    const Vec3<f32>& normal,
    Prng& prng
) noexcept {
    f32 u1 = prng.next_f32();
    f32 u2 = prng.next_f32();
    f32 r = std::sqrt(u1);
    f32 phi = 2.0F * PI * u2;
    auto [tangent, bitangent] = make_orthonormal_basis(normal);
    return tangent * (r * std::cos(phi)) + bitangent * (r * std::sin(phi))
        + normal * std::sqrt(std::max(1.0F - u1, 0.0F));
}

/**
 * @brief Samples a direction inside a cone.
 *
 * @param[in]      axis   The axis of the cone (normalized).
 * @param[in]      cosMax The cosine of the half-angle of the cone.
 * @param[in, out] prng   A pseudorandom number generator.
 * @return A direction inside the cone.
 */
static constexpr Vec3<f32> sample_cone(
    const Vec3<f32>& axis,
    f32 cosMax,
    Prng& prng
) noexcept {
    f32 u1 = prng.next_f32();
    f32 u2 = prng.next_f32();
    f32 cosTheta = 1.0F - u1 * (1.0F - cosMax);
    f32 sinTheta = std::sqrt(std::max(1.0F - cosTheta * cosTheta, 0.0F));
    f32 phi = 2.0F * PI * u2;
    auto [tangent, bitangent] = make_orthonormal_basis(axis);
    return tangent * (sinTheta * std::cos(phi))
        + bitangent * (sinTheta * std::sin(phi)) + axis * cosTheta;
}

/**
 * @brief Samples a spherical light source.
 *
 * @param[in]      light       The position of the light.
 * @param[in]      radius      The radius of the light.
 * @param[in]      attenuation The attenuation factors of the light.
 * @param[in]      position    The position at which to sample.
 * @param[in, out] prng        A pseudorandom number generator.
 * @return The sample of the spherical light source.
 */
static constexpr LightSample sample_sphere_light(
    const Vec3<f32>& light,
    f32 radius,
    const Light::Attenuation& attenuation,
    const Vec3<f32>& position,
    Prng& prng
) noexcept {
    Vec3<f32> direction = light - position;
    f32 distance = direction.norm();
    direction /= distance;
    LightSample sample = {
        .direction = direction,
        .distance = distance,
        .radiance = Color::splat(1.0F)
    };
    if ((radius > 0.0F) && (distance > radius)) {
        f32 sinMax = radius / distance;
        sample.direction = sample_cone(
            direction,
            std::sqrt(1.0F - sinMax * sinMax),
            prng
        );
        f32 b = distance * sample.direction.dot(direction);
        f32 d = b * b - (distance * distance - radius * radius);
        sample.distance = b - std::sqrt(std::max(d, 0.0F));
    }
    const auto& [c, l, q] = attenuation;
    sample.radiance /= c + l * distance + q * distance * distance;
    return sample;
}

/**
 * @brief Returns the contribution of a light for direct lighting.
 *
 * @param[in]      light    The light to sample.
 * @param[in]      position The position at which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The contribution of the light for direct lighting.
 */
static constexpr LightSample sample_light(
    const Light& light,
    const Vec3<f32>& position,
    Prng& prng
) noexcept {
    Color emitted = light.color() * light.intensity();
    return std::visit(
        Overload {
            [&](const Light::Point& point) {
                LightSample sample = sample_sphere_light(
                    point.position,
                    point.radius,
                    point.attenuation,
                    position,
                    prng
                );
                sample.radiance *= emitted;
                return sample;
            },
            [&](const Light::Spot& spot) {
                LightSample sample = sample_sphere_light(
                    spot.position,
                    spot.radius,
                    spot.attenuation,
                    position,
                    prng
                );
                f32 cosAngle = (position - spot.position).normalize()
                    .dot(spot.direction.normalize());
                f32 t = std::clamp(
                    (cosAngle - spot.outerCutoff)
                        / std::max(spot.innerCutoff - spot.outerCutoff, 1.0E-6F),
                    0.0F,
                    1.0F
                );
                sample.radiance *= emitted * (t * t * (3.0F - 2.0F * t));
                return sample;
            },
            [&](const Light::Directional& directional) {
                Vec3<f32> direction = (-directional.direction).normalize();
                if (directional.angularRadius < 1.0F) {
                    direction = sample_cone(
                        direction,
                        directional.angularRadius,
                        prng
                    );
                }
                return LightSample {
                    .direction = direction,
                    .distance = INF,
                    .radiance = emitted
                };
            } },
        light.kind()
    );
}

/**
 * @brief Offsets a position along a normal to avoid self-intersection.
 *
 * @param[in] position The position to offset.
 * @param[in] normal   The normal along which to offset (normalized).
 * @return The position offset along the normal.
 */
static constexpr Vec3<f32> offset_position(
    const Vec3<f32>& position,
    const Vec3<f32>& normal
) noexcept {
    constexpr f32 ORIGIN = 1.0F / 32.0F;
    constexpr f32 FLOAT_SCALE = 1.0F / 65536.0F;
    constexpr f32 INT_SCALE = 256.0F;
    auto nudge = [&](f32 p, f32 n) {
        if (std::abs(p) < ORIGIN) {
            return p + FLOAT_SCALE * n;
        }
        i32 offset = static_cast<i32>(INT_SCALE * n);
        i32 bits = std::bit_cast<i32>(p);
        return std::bit_cast<f32>(bits + ((p < 0.0F) ? -offset : offset));
    };
    return {
        .x = nudge(position.x, normal.x),
        .y = nudge(position.y, normal.y),
        .z = nudge(position.z, normal.z)
    };
}

/**
 * @brief Returns the diffuse color of a material.
 *
 * @param[in] material The material to evaluate.
 * @return The diffuse color of the material.
 */
static constexpr Color diffuse_color(const Material& material) noexcept {
    if (auto opaque = material.as_opaque()) {
        return material.albedo() * (1.0F - opaque->metalness);
    }
    return material.albedo();
}

/**
 * @brief Determines if a material is a perfect mirror.
 *
 * @param[in] material The material to evaluate.
 * @return `true` if the material is a perfect mirror, otherwise `false`.
 */
static constexpr bool is_perfect_mirror(const Material& material) noexcept {
    return material.roughness() == 0.0F;
}

/**
 * @brief Returns the GGX alpha of a material.
 *
 * @param[in] material The material to evaluate.
 * @return The GGX alpha of the material.
 */
static constexpr f32 ggx_alpha(const Material& material) noexcept {
    return std::max(material.roughness() * material.roughness(), MIN_ALPHA);
}

/**
 * @brief Returns the fresnel reflectance using Schlick's approximation.
 *
 * @param[in] f0       The reflectance at normal incidence.
 * @param[in] cosTheta The cosine of the angle between the two directions.
 * @return The fresnel reflectance using Schlick's approximation.
 */
static constexpr Color fresnel_schlick(const Color& f0, f32 cosTheta) noexcept {
    f32 x = std::clamp(1.0F - cosTheta, 0.0F, 1.0F);
    return f0 + (Color::splat(1.0F) - f0) * (x * x * x * x * x);
}

/**
 * @brief Returns the fresnel reflectance of a dielectric interface.
 *
 * @param[in] cosI The cosine of the incident angle.
 * @param[in] etaI The index of refraction on the incident side.
 * @param[in] etaT The index of refraction on the transmitted side.
 * @return The fresnel reflectance and the cosine of the transmitted angle.
 */
static constexpr std::pair<f32, f32> fresnel_dielectric(
    f32 cosI,
    f32 etaI,
    f32 etaT
) noexcept {
    f32 eta = etaI / etaT;
    f32 sin2T = eta * eta * (1.0F - cosI * cosI);
    if (sin2T >= 1.0F) {
        return { 1.0F, 0.0F };
    }
    f32 cosT = std::sqrt(1.0F - sin2T);
    f32 a = etaI * cosI;
    f32 b = etaT * cosT;
    f32 c = etaT * cosI;
    f32 d = etaI * cosT;
    f32 rs = (a - b) / std::max(a + b, 1.0E-12F);
    f32 rp = (c - d) / std::max(c + d, 1.0E-12F);
    return { 0.5F * (rs * rs + rp * rp), cosT };
}

/**
 * @brief Returns the indices of refraction on the incident and transmitted side
 * of a hit.
 *
 * @param[in] hit         The hit information.
 * @param[in] transparent The properties of the transparent material.
 * @param[in] media       The medium stack.
 * @return The indices of refraction on the incident and transmitted side.
 */
static constexpr std::pair<f32, f32> interface_iors(
    const Hit& hit,
    const Material::Transparent& transparent,
    const MediumStack& media
) noexcept {
    if (hit.entering) {
        return { media.ior(), transparent.ior };
    }
    return { transparent.ior, media.surrounding_ior(hit.material) };
}

/**
 * @brief Samples a dielectric material.
 *
 * @param[in]      hit         The hit information.
 * @param[in]      transparent The properties of the transparent material.
 * @param[in]      media       The medium stack.
 * @param[in]      view        The direction towards the camera (normalized).
 * @param[in, out] prng        A pseudorandom number generator.
 * @return The sampled bounce for the dielectric material.
 */
static constexpr Bounce sample_dielectric(
    const Hit& hit,
    const Material::Transparent& transparent,
    const MediumStack& media,
    const Vec3<f32>& view,
    Prng& prng
) noexcept {
    auto [etaI, etaT] = interface_iors(hit, transparent, media);
    f32 cosI = std::clamp(hit.normal.dot(view), 0.0F, 1.0F);
    auto [fresnel, cosT] = fresnel_dielectric(cosI, etaI, etaT);
    if (prng.next_f32() < fresnel) {
        return {
            .direction = view.reflect(hit.normal),
            .weight = Color::splat(1.0F)
        };
    }
    f32 eta = etaI / etaT;
    return {
        .direction = (-view * eta + hit.normal * (eta * cosI - cosT))
            .normalize(),
        .weight = Color::splat(1.0F),
        .transmitted = true
    };
}

/**
 * @brief Returns the fraction of light that survives while traveling through a
 * medium.
 *
 * @param[in] absorption The absorption of the medium.
 * @param[in] distance   The distance traveled through the medium.
 * @return The fraction of light that survives while traveling through the
 * medium.
 */
static constexpr Color beer_lambert(
    const Color& absorption,
    f32 distance
) noexcept {
    return Color {
        .r = std::exp(-absorption.r * distance),
        .g = std::exp(-absorption.g * distance),
        .b = std::exp(-absorption.b * distance)
    };
}

/**
 * @brief Returns the GGX normal distribution.
 *
 * @param[in] normal The surface normal (normalized).
 * @param[in] half   The half vector between view and light directions
 *                   (normalized).
 * @param[in] alpha  The GGX alpha of the material.
 * @return The density of microfacet normals aligned with the half vector.
 */
static constexpr f32 ggx_distribution(
    const Vec3<f32>& normal,
    const Vec3<f32>& half,
    f32 alpha
) noexcept {
    Vec3<f32> nCrossH = normal.cross(half);
    f32 nDotH = normal.dot(half);
    f32 k = alpha / (nCrossH.dot(nCrossH) + nDotH * nDotH * alpha * alpha);
    return k * k / PI;
}

/**
 * @brief Returns the Smith-GGX denominator for a single direction.
 *
 * @param[in] nDotX The cosine of the angle between the normal and direction.
 * @param[in] a2    The square of the GGX alpha of the material.
 * @return The Smith-GGX denominator for the direction.
 */
static constexpr f32 smith_denominator(f32 nDotX, f32 a2) noexcept {
    return nDotX + std::sqrt(a2 + (1.0F - a2) * nDotX * nDotX);
}

/**
 * @brief Returns the BRDF using Lambert diffuse and Cook-Torrance specular.
 *
 * @param[in] material The material of the surface.
 * @param[in] normal   The surface normal (normalized).
 * @param[in] view     The direction towards the camera (normalized).
 * @param[in] light    The direction towards the light (normalized).
 * @return The BRDF for the specified directions.
 */
static constexpr Color brdf(
    const Material& material,
    const Vec3<f32>& normal,
    const Vec3<f32>& view,
    const Vec3<f32>& light
) noexcept {
    Vec3<f32> half = (view + light).normalize();
    Color f = fresnel_schlick(
        material.f0(),
        is_perfect_mirror(material) ? normal.dot(view) : view.dot(half)
    );
    Color diffuse = diffuse_color(material) * (Color::splat(1.0F) - f) / PI;
    if (is_perfect_mirror(material)) {
        return diffuse;
    }
    f32 alpha = ggx_alpha(material);
    f32 a2 = alpha * alpha;
    f32 nDotV = std::max(normal.dot(view), 0.0F);
    f32 nDotL = std::max(normal.dot(light), 0.0F);
    f32 specular = ggx_distribution(normal, half, alpha)
        / (smith_denominator(nDotV, a2) * smith_denominator(nDotL, a2));
    return diffuse + f * specular;
}

/**
 * @brief Samples a microfacet normal from the distribution of normals visible
 * from a direction.
 *
 * @param[in]      normal The surface normal (normalized).
 * @param[in]      view   The direction towards the camera (normalized).
 * @param[in]      alpha  The GGX alpha of the material.
 * @param[in, out] prng   A pseudorandom number generator.
 * @return A microfacet normal (normalized).
 */
static constexpr Vec3<f32> sample_ggx_normal(
    const Vec3<f32>& normal,
    const Vec3<f32>& view,
    f32 alpha,
    Prng& prng
) noexcept {
    auto [tangent, bitangent] = make_orthonormal_basis(normal);
    Vec3<f32> stretched = {
        .x = alpha * view.dot(tangent),
        .y = alpha * view.dot(bitangent),
        .z = view.dot(normal)
    };
    stretched = stretched.normalize();
    f32 length2 = stretched.x * stretched.x + stretched.y * stretched.y;
    Vec3<f32> t1 = (length2 > 0.0F)
        ? Vec3<f32> { -stretched.y, stretched.x, 0.0F } / std::sqrt(length2)
        : Vec3<f32> { 1.0F, 0.0F, 0.0F };
    Vec3<f32> t2 = stretched.cross(t1);
    f32 u1 = prng.next_f32();
    f32 u2 = prng.next_f32();
    f32 r = std::sqrt(u1);
    f32 phi = 2.0F * PI * u2;
    f32 p1 = r * std::cos(phi);
    f32 s = 0.5F * (1.0F + stretched.z);
    f32 p2 = (1.0F - s) * std::sqrt(1.0F - p1 * p1) + s * r * std::sin(phi);
    Vec3<f32> h = t1 * p1 + t2 * p2
        + stretched * std::sqrt(std::max(1.0F - p1 * p1 - p2 * p2, 0.0F));
    Vec3<f32> local = {
        .x = alpha * h.x,
        .y = alpha * h.y,
        .z = std::max(h.z, 0.0F)
    };
    local = local.normalize();
    return tangent * local.x + bitangent * local.y + normal * local.z;
}

/**
 * @brief Samples the diffuse or specular lobe of a material.
 *
 * @param[in]      material The material of the surface.
 * @param[in]      normal   The surface normal (normalized).
 * @param[in]      view     The direction towards the camera (normalized).
 * @param[in, out] prng     A pseudorandom number generator.
 * @return The sampled bounce (with a weight of zero if it is not usable).
 */
static constexpr Bounce sample_opaque(
    const Material& material,
    const Vec3<f32>& normal,
    const Vec3<f32>& view,
    Prng& prng
) noexcept {
    Color diffuse = diffuse_color(material);
    Color reflectance = fresnel_schlick(material.f0(), normal.dot(view));
    f32 specularEnergy = reflectance.max();
    f32 diffuseEnergy = diffuse.max() * (1.0F - specularEnergy);
    f32 totalEnergy = specularEnergy + diffuseEnergy;
    if (totalEnergy <= 0.0F) {
        return { .direction = normal, .weight = Color::splat(0.0F) };
    }
    f32 specularProbability = specularEnergy / totalEnergy;
    f32 diffuseProbability = diffuseEnergy / totalEnergy;
    if (prng.next_f32() < specularProbability) {
        if (is_perfect_mirror(material)) {
            return {
                .direction = view.reflect(normal),
                .weight = reflectance / specularProbability
            };
        }
        f32 alpha = ggx_alpha(material);
        Vec3<f32> h = sample_ggx_normal(normal, view, alpha, prng);
        Vec3<f32> l = view.reflect(h);
        f32 nDotL = normal.dot(l);
        if (nDotL <= 0.0F) {
            return { .direction = l, .weight = Color::splat(0.0F) };
        }
        return {
            .direction = l,
            .weight = fresnel_schlick(material.f0(), view.dot(h))
                * (2.0F * nDotL / smith_denominator(nDotL, alpha * alpha))
                / specularProbability
        };
    }
    if (diffuseProbability <= 0.0F) {
        return { .direction = normal, .weight = Color::splat(0.0F) };
    }
    Vec3<f32> direction = sample_cosine_hemisphere(normal, prng);
    Vec3<f32> half = (view + direction).normalize();
    Color f = fresnel_schlick(
        material.f0(),
        is_perfect_mirror(material) ? normal.dot(view) : view.dot(half)
    );
    return {
        .direction = direction,
        .weight = diffuse * (Color::splat(1.0F) - f) / diffuseProbability
    };
}

/**
 * @brief Returns the fraction of light that survives a shadow ray.
 *
 * @param[in] scene The scene to trace the shadow ray through.
 * @param[in] ray   The shadow ray.
 * @param[in] media The media at the origin of the ray.
 * @return The fraction of light that survives the shadow ray.
 */
static constexpr Color shadow_transmittance(
    const Scene& scene,
    Ray ray,
    MediumStack media
) noexcept {
    Color transmittance = Color::splat(1.0F);
    for (isize i = 0; i < MAX_SHADOW_CROSSINGS; i++) {
        std::optional<Hit> hit = intersect_scene(scene, ray);
        if (!hit) {
            if (!media.empty() && (ray.maxDistance != INF)) {
                transmittance *= beer_lambert(
                    media.absorption(),
                    std::max(ray.maxDistance, 0.0F)
                );
            }
            return transmittance;
        }
        auto transparent = hit->material->as_transparent();
        if (!transparent) {
            return Color::splat(0.0F);
        }
        if (!media.empty()) {
            transmittance *= beer_lambert(media.absorption(), hit->distance);
        }
        auto [etaI, etaT] = interface_iors(*hit, *transparent, media);
        f32 cosI = std::clamp(hit->normal.dot(-ray.direction), 0.0F, 1.0F);
        auto [fresnel, _] = fresnel_dielectric(
            cosI,
            std::min(etaI, etaT),
            std::max(etaI, etaT)
        );
        transmittance *= transparent->transparency * (1.0F - fresnel);
        media.transition(*hit);
        ray.origin = offset_position(hit->position, -hit->normal);
        ray.maxDistance -= hit->distance;
    }
    return Color::splat(0.0F);
}

/**
 * @brief Returns the direct lighting at a specified position.
 *
 * @param[in]      scene The scene containing lights and spheres.
 * @param[in]      hit   The position at which to compute the lighting.
 * @param[in]      media The media at the hit.
 * @param[in]      view  The direction towards the camera (normalized).
 * @param[in, out] prng  A pseudorandom number generator.
 * @return The direct lighting at the specified position.
 */
static constexpr Color direct_lighting(
    const Scene& scene,
    const Hit& hit,
    const MediumStack& media,
    const Vec3<f32>& view,
    Prng& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    for (const Light& light : scene.lights) {
        LightSample sample = sample_light(light, hit.position, prng);
        f32 cosTheta = hit.normal.dot(sample.direction);
        if (cosTheta <= 0.0F) {
            continue;
        }
        Ray shadowRay = {
            .origin = offset_position(hit.position, hit.normal),
            .direction = sample.direction,
            .minDistance = 0.0F,
            .maxDistance = sample.distance
        };
        Color visibility = shadow_transmittance(scene, shadowRay, media);
        if (visibility.max() <= 0.0F) {
            continue;
        }
        radiance += sample.radiance * visibility
            * brdf(*hit.material, hit.normal, view, sample.direction)
            * cosTheta;
    }
    return radiance;
}

/**
 * @brief Returns the color obtained by tracing a ray through a pixel.
 *
 * @param[in]      scene The scene to trace the ray through.
 * @param[in]      ray   The ray to trace.
 * @param[in]      media The media through which the ray is traced.
 * @param[in, out] prng  A pseudorandom number generator.
 * @return The color obtained by tracing a ray through a pixel.
 */
static constexpr Color trace_ray(
    const Scene& scene,
    Ray ray,
    MediumStack media,
    Prng& prng
) noexcept {
    Color radiance = Color::splat(0.0F);
    Color throughput = Color::splat(1.0F);
    isize opaqueHits = 0;
    for (isize depth = 0; depth < MAX_DEPTH; depth++) {
        std::optional<Hit> hit = intersect_scene(scene, ray);
        if (!hit) {
            radiance += throughput * sky_color(ray.direction);
            break;
        }
        if (!media.empty()) {
            throughput *= beer_lambert(media.absorption(), hit->distance);
        }
        Vec3<f32> view = -ray.direction;
        auto transparent = hit->material->as_transparent();
        bool useDielectric = transparent
            && (prng.next_f32() < transparent->transparency);
        Bounce bounce = useDielectric
            ? sample_dielectric(*hit, *transparent, media, view, prng)
            : sample_opaque(*hit->material, hit->normal, view, prng);
        if (!useDielectric) {
            Color direct = throughput
                * direct_lighting(scene, *hit, media, view, prng);
            f32 directMax = direct.max();
            if ((opaqueHits > 0) && (directMax > MAX_INDIRECT)) {
                direct *= MAX_INDIRECT / directMax;
            }
            radiance += direct;
            opaqueHits++;
        }
        throughput *= bounce.weight;
        f32 throughputMax = throughput.max();
        if (throughputMax <= 0.0F) {
            break;
        }
        if (depth >= RUSSIAN_ROULETTE_DEPTH) {
            f32 survival = std::clamp(throughputMax, 0.05F, 1.0F);
            if (prng.next_f32() >= survival) {
                break;
            }
            throughput /= survival;
        }
        if (bounce.transmitted) {
            media.transition(*hit);
        }
        ray = {
            .origin = offset_position(
                hit->position,
                (bounce.direction.dot(hit->normal) < 0.0F)
                    ? -hit->normal
                    : hit->normal
            ),
            .direction = bounce.direction,
            .minDistance = 0.0F,
            .maxDistance = INF
        };
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
    MediumStack media = initial_media(scene, scene.camera.position());
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
                        .origin = scene.camera.position(),
                        .direction = (viewToWorld * direction).normalize(),
                        .minDistance = 0.0F,
                        .maxDistance = INF
                    };
                    Color color = trace_ray(scene, ray, media, prng);
                    film.add_sample(x, y, color);
                }
            }
        );
    }
    threadPool.wait_idle();
    film.end_frame();
}

}