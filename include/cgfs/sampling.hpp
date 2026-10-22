#ifndef CGFS_SAMPLING_HPP
#define CGFS_SAMPLING_HPP

#include <algorithm>
#include <cmath>
#include "cgfs/basis.hpp"
#include "cgfs/prng.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec2.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A sampled point on an object as seen from a position. */
struct PointSample {

    /** @brief The direction to the sampled point. */
    Vec3<f32> direction;

    /** @brief The distance to the sampled point. */
    f32 distance;

    /** @brief The solid-angle PDF (if available). */
    std::optional<f32> pdf;

};

/**
 * @brief Samples a point uniformly on the unit disk.
 *
 * @param[in, out] prng A pseudorandom number generator.
 * @return A uniformly sampled point on the unit disk.
 */
constexpr Vec2<f32> sample_unit_disk(Xoshiro128StarStar& prng) noexcept {
    f32 r = std::sqrt(prng.next_f32());
    f32 phi = 2.0F * PI<f32> * prng.next_f32();
    return { .x = r * std::cos(phi), .y = r * std::sin(phi) };
}

/**
 * @brief Samples a cosine-weighted direction in a hemisphere around a normal.
 *
 * @param[in]      normal The normal around which to sample (normalized).
 * @param[in, out] prng   A pseudorandom number generator.
 * @return A cosine-weighted direction in the hemisphere around the normal.
 */
constexpr Vec3<f32> sample_cosine_hemisphere(
    const Vec3<f32>& normal,
    Xoshiro128StarStar& prng
) noexcept {
    auto [x, y] = sample_unit_disk(prng);
    Vec3<f32> local = {
        .x = x,
        .y = y,
        .z = std::sqrt(std::max(1.0F - x * x - y * y, 0.0F))
    };
    return Basis::around(normal).to_world(local);
}

/**
 * @brief Returns the versine of the half-angle of a cone subtended by a sphere.
 *
 * @param[in] distanceSquared The squared distance to the center of the sphere.
 * @param[in] radius          The radius of the sphere.
 * @return The versine of the half-angle of the cone subtended by the sphere.
 */
constexpr f32 sphere_versine(f32 distanceSquared, f32 radius) noexcept {
    f32 sinSquared = std::min(radius * radius / distanceSquared, 1.0F);
    return sinSquared / (1.0F + std::sqrt(1.0F - sinSquared));
}

/**
 * @brief Returns the solid-angle PDF of sampling a cone uniformly.
 *
 * @param[in] versine The versine of the half-angle of the cone.
 * @return The solid-angle PDF of sampling the cone uniformly.
 */
constexpr f32 cone_pdf(f32 versine) noexcept {
    return 1.0F / (2.0F * PI<f32> * versine);
}

/**
 * @brief Samples a direction uniformly inside a cone.
 *
 * @param[in]      axis    The axis of the cone (normalized).
 * @param[in]      versine The versine of the half-angle of the cone.
 * @param[in, out] prng    A pseudorandom number generator.
 * @return A uniformly sampled direction inside the cone.
 */
constexpr Vec3<f32> sample_cone(
    const Vec3<f32>& axis,
    f32 versine,
    Xoshiro128StarStar& prng
) noexcept {
    auto [x, y] = sample_unit_disk(prng);
    f32 s = x * x + y * y;
    f32 scale = std::sqrt(std::max(versine * (2.0F - versine * s), 0.0F));
    Vec3<f32> local = {
        .x = x * scale,
        .y = y * scale,
        .z = 1.0F - versine * s
    };
    return Basis::around(axis).to_world(local);
}

/**
 * @brief Samples a direction towards a sphere, uniformly by solid angle.
 *
 * @param[in]      center   The center of the sphere.
 * @param[in]      radius   The radius of the sphere.
 * @param[in]      position The position from which to sample.
 * @param[in, out] prng     A pseudorandom number generator.
 * @return A sample on success, or `std::nullopt` if the position is inside the
 * sphere.
 */
static constexpr std::optional<PointSample> sample_sphere(
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
    f32 versine = sphere_versine(distanceSquared, radius);
    Vec3<f32> direction = sample_cone(
        axis * (1.0F / std::sqrt(distanceSquared)),
        versine,
        prng
    );
    f32 b = direction.dot(axis);
    f32 d = b * b - (distanceSquared - radiusSquared);
    f32 distance = b - std::sqrt(std::max(d, 0.0F));
    return PointSample {
        .direction = direction,
        .distance = distance,
        .pdf = cone_pdf(versine)
    };
}

}

#endif