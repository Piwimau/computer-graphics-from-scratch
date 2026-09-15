#ifndef CGFS_LIGHT_HPP
#define CGFS_LIGHT_HPP

#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents an ambient light with a constant intensity. */
struct AmbientLight {

    /** @brief The intensity of this ambient light. */
    f64 intensity;

};

/** @brief Represents a point light radiating from a fixed position. */
struct PointLight {

    /** @brief The intensity of this light. */
    f64 intensity;

    /** @brief The position of this light. */
    Vec3<f64> pos;

};

/** @brief Represents a directional light radiating in a fixed direction. */
struct DirectionalLight {

    /** @brief The intensity of this light. */
    f64 intensity;

    /** @brief The direction of this light. */
    Vec3<f64> dir;

};

}

#endif