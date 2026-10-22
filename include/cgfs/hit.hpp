#ifndef CGFS_HIT_HPP
#define CGFS_HIT_HPP

#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief An intersection between a ray and an object. */
struct Hit {

    /** @brief The distance along the ray to the hit. */
    f32 distance;

    /** @brief The position of the hit. */
    Vec3<f32> position;

    /** @brief The surface normal at the hit. */
    Vec3<f32> normal;

    /** @brief The index of the object that was hit. */
    isize idx;

    /** @brief Whether the ray hit the object from outside. */
    bool entering;

};

}

#endif