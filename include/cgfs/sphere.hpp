#ifndef CGFS_SPHERE_HPP
#define CGFS_SPHERE_HPP

#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents a sphere. */
struct Sphere {

    /** @brief The center of this sphere. */
    Vec3<f32> center;

    /** @brief The radius of this sphere. */
    f32 radius;

    /** @brief The color of this sphere. */
    Color color;

};

}

#endif