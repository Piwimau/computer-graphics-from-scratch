#ifndef CGFS_TRIANGLE_HPP
#define CGFS_TRIANGLE_HPP

#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a triangle. */
struct Triangle {

    /** @brief The first vertex of this triangle. */
    Vec3<f32> v0;

    /** @brief The second vertex of this triangle. */
    Vec3<f32> v1;

    /** @brief The third vertex of this triangle. */
    Vec3<f32> v2;

    /**
     * @brief Returns the normalized surface normal of this triangle.
     *
     * @return The normalized surface normal of this triangle.
     */
    constexpr Vec3<f32> normal() const noexcept {
        return (v1 - v0).cross(v2 - v0).normalize();
    }

};

}

#endif