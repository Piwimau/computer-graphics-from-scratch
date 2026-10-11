#ifndef CGFS_BASIS_HPP
#define CGFS_BASIS_HPP

#include <cmath>
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief An orthonormal basis around a normal. */
struct Basis {

    /** @brief The tangent (normalized). */
    Vec3<f32> tangent;

    /** @brief The bitangent (normalized). */
    Vec3<f32> bitangent;

    /** @brief The normal (normalized). */
    Vec3<f32> normal;

    /**
     * @brief Constructs an orthonormal basis around a normal.
     *
     * @param[in] normal The normal around which to build the basis
     *                   (normalized).
     * @return The orthonormal basis around the normal.
     *
     * @warning The behavior is undefined if `normal` is not normalized.
     */
    static constexpr Basis around(const Vec3<f32>& normal) noexcept {
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
        return { tangent, bitangent, normal };
    }

    /**
     * @brief Transforms a world-space vector to local space.
     *
     * @param[in] world The world-space vector to transform.
     * @return The local-space vector.
     */
    constexpr Vec3<f32> to_local(const Vec3<f32>& world) const noexcept {
        return {
            .x = world.dot(tangent),
            .y = world.dot(bitangent),
            .z = world.dot(normal)
        };
    }

    /**
     * @brief Transforms a local-space vector to world space.
     *
     * @param[in] local The local-space vector to transform.
     * @return The world-space vector.
     */
    constexpr Vec3<f32> to_world(const Vec3<f32>& local) const noexcept {
        return tangent * local.x + bitangent * local.y + normal * local.z;
    }

};

}

#endif