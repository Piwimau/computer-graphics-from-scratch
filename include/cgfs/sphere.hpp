#ifndef CGFS_SPHERE_HPP
#define CGFS_SPHERE_HPP

#include <optional>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents a sphere. */
struct Sphere {

    /** @brief The center of this sphere. */
    Vec3<f64> center;

    /** @brief The radius of this sphere. */
    f64 radius;

    /** @brief The color of this sphere. */
    Color color;

    /** @brief The shininess of this sphere (if any). */
    std::optional<f64> shininess;

    /** @brief The reflectiveness of this sphere (if any). */
    std::optional<f64> reflectiveness;

};

}

#endif