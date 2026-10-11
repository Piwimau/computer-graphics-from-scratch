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
constexpr Vec2<f32> sample_unit_disk(Prng& prng) noexcept {
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
    Prng& prng
) noexcept {
    auto [x, y] = sample_unit_disk(prng);
    Vec3<f32> local = {
        .x = x,
        .y = y,
        .z = std::sqrt(std::max(1.0F - x * x - y * y, 0.0F))
    };
    return Basis::around(normal).to_world(local);
}

}

#endif