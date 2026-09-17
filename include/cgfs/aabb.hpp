#ifndef CGFS_AABB_HPP
#define CGFS_AABB_HPP

#include <cassert>
#include <ranges>
#include <span>
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents an axis-aligned bounding box. */
struct Aabb {

    /** @brief The lower corner of this bounding box. */
    Vec3<f32> min;

    /** @brief The upper corner of this bounding box. */
    Vec3<f32> max;

    /**
     * @brief Creates an axis-aligned bounding box enclosing a set of points.
     *
     * @warning The behavior is undefined if `points` is empty.
     *
     * @param[in] points The set of points to enclose.
     * @return An axis-aligned bounding box enclosing the set of points.
     */
    static constexpr Aabb create(std::span<const Vec3<f32>> points) noexcept {
        assert(!points.empty());
        Vec3<f32> min = points[0];
        Vec3<f32> max = points[0];
        for (const Vec3<f32>& point : std::views::drop(points, 1)) {
            min = min.min(point);
            max = max.max(point);
        }
        return { min, max };
    }

};

}

#endif