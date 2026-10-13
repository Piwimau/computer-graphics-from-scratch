#ifndef CGFS_SAMPLING_HPP
#define CGFS_SAMPLING_HPP

#include <algorithm>
#include <cmath>
#include "basis.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec2.hpp"
#include "cgfs/vec3.hpp"
#include "prng.hpp"

namespace cgfs {

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
 * @brief Returns the solid-angle PDF of sampling a cone subtended by a sphere
 * uniformly.
 *
 * @param[in] center   The center of the sphere.
 * @param[in] radius   The radius of the sphere.
 * @param[in] position The position from which the cone is subtended.
 * @return The solid-angle PDF of sampling the cone subtended by the sphere
 * uniformly.
 */
constexpr f32 sphere_cone_pdf(
    const Vec3<f32>& center,
    f32 radius,
    const Vec3<f32>& position
) noexcept {
    f32 distanceSquared = (center - position).norm_squared();
    f32 versine = sphere_versine(distanceSquared, radius);
    return cone_pdf(versine);
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

}

#endif