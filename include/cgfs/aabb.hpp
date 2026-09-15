#ifndef CGFS_AABB_HPP
#define CGFS_AABB_HPP

#include <algorithm>
#include <cassert>
#include <ranges>
#include <span>
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents an axis-aligned bounding box. */
struct Aabb {

    /** @brief The lower corner of this bounding box. */
    Vec3<f64> min;

    /** @brief The upper corner of this bounding box. */
    Vec3<f64> max;

    /**
     * @brief Creates an axis-aligned bounding box enclosing a set of points.
     *
     * @param[in] points The set of points to enclose.
     * @return An axis-aligned bounding box enclosing the set of points.
     */
    static constexpr Aabb enclosing(
        std::span<const Vec3<f64>> points
    ) noexcept {
        assert(!points.empty());
        Vec3<f64> min = points[0];
        Vec3<f64> max = points[0];
        for (const Vec3<f64>& point : std::views::drop(points, 1)) {
            min = {
                std::min(min.x, point.x),
                std::min(min.y, point.y),
                std::min(min.z, point.z)
            };
            max = {
                std::max(max.x, point.x),
                std::max(max.y, point.y),
                std::max(max.z, point.z)
            };
        }
        return { min, max };
    }

};

}

#endif