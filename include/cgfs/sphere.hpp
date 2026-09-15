#ifndef CGFS_SPHERE_HPP
#define CGFS_SPHERE_HPP

#include "cgfs/material.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a sphere. */
struct Sphere {

    /** @brief The center of this sphere. */
    Vec3<f64> center;

    /** @brief The radius of this sphere. */
    f64 radius;

    /** @brief The material of this sphere. */
    Material material;

};

}

#endif