#ifndef CGFS_CONE_HPP
#define CGFS_CONE_HPP

#include <algorithm>
#include <cmath>
#include "basis.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"
#include "prng.hpp"
#include "sampling.hpp"

namespace cgfs {

/** @brief A cone of directions. */
struct Cone {

    /** @brief The axis (normalized). */
    Vec3<f32> axis;

    /** @brief The cosine of the half-angle. */
    f32 cosMax;

    /** @brief The sine of the half-angle. */
    f32 sinMax;

    /**
     * @brief Constructs a cone from a sphere and position.
     *
     * @param[in] center   The center of the sphere.
     * @param[in] radius   The radius of the sphere.
     * @param[in] position The position from which the sphere is seen.
     * @return The cone.
     */
    static constexpr Cone from(
        const Vec3<f32>& center,
        f32 radius,
        const Vec3<f32>& position
    ) noexcept {
        Vec3<f32> axis = center - position;
        f32 distance = axis.norm();
        f32 sinMax = radius / distance;
        return {
            .axis = axis / distance,
            .cosMax = std::sqrt(std::max(1.0F - sinMax * sinMax, 0.0F)),
            .sinMax = sinMax
        };
    }

    /**
     * @brief Constructs a cone from an axis and cosine of its half-angle.
     *
     * @param[in] axis   The axis of the cone (normalized).
     * @param[in] cosMax The cosine of the half-angle of the cone.
     * @return The cone.
     */
    static constexpr Cone from(const Vec3<f32>& axis, f32 cosMax) noexcept {
        return {
            .axis = axis,
            .cosMax = cosMax,
            .sinMax = std::sqrt(std::max(1.0F - cosMax * cosMax, 0.0F))
        };
    }

    /**
     * @brief Returns the versine of the half-angle of this cone.
     *
     * @return The versine of the half-angle of this cone.
     */
    constexpr f32 versine() const noexcept {
        return sinMax * sinMax / (1.0F + cosMax);
    }

    /**
     * @brief Returns the solid-angle PDF of sampling this cone uniformly.
     *
     * @return The solid-angle PDF of sampling this cone uniformly.
     */
    constexpr f32 pdf() const noexcept {
        return 1.0F / (2.0F * PI<f32> * versine());
    }

    /**
     * @brief Samples a direction uniformly inside this cone.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return A uniformly sampled direction inside this cone.
     */
    constexpr Vec3<f32> sample(Prng& prng) const noexcept {
        auto [x, y] = sample_unit_disk(prng);
        f32 k = versine();
        f32 s = x * x + y * y;
        f32 scale = std::sqrt(k * (2.0F - k * s));
        Vec3<f32> local = {
            .x = x * scale,
            .y = y * scale,
            .z = 1.0F - k * s
        };
        return Basis::around(axis).to_world(local);
    }

};

}

#endif