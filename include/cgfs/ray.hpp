#ifndef CGFS_RAY_HPP
#define CGFS_RAY_HPP

#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A ray. */
struct Ray {

    /** @brief The origin in world space. */
    Vec3<f32> origin;

    /** @brief The direction in world space. */
    Vec3<f32> direction;

    /** @brief The maximum distance for intersection tests. */
    f32 maxDistance;

};

}

#endif