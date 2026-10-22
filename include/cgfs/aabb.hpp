#ifndef CGFS_AABB_HPP
#define CGFS_AABB_HPP

#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief An axis-aligned bounding box. */
struct Aabb {

    /** @brief The minimum corner. */
    Vec3<f32> min;

    /** @brief The maximum corner. */
    Vec3<f32> max;

};

}

#endif