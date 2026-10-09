#ifndef CGFS_SPHERE_HPP
#define CGFS_SPHERE_HPP

#include "cgfs/material.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A sphere. */
struct Sphere {

    /** @brief The position in world space. */
    Vec3<f32> center;

    /** @brief The radius. */
    f32 radius;

    /** @brief The surface material. */
    Material material;

};

}

#endif