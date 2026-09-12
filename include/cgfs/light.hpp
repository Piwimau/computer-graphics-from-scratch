#ifndef CGFS_LIGHT_HPP
#define CGFS_LIGHT_HPP

#include <variant>
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents an ambient light with a constant intensity. */
struct AmbientLight {

    /** @brief The intensity of this ambient light. */
    f64 intensity;

};

/** @brief Represents a point light radiating from a fixed position. */
struct PointLight {

    /** @brief The intensity of this point light. */
    f64 intensity;

    /** @brief The position of this point light. */
    Vec3<f64> position;

};

/** @brief Represents a directional light radiating in a fixed direction. */
struct DirectionalLight {

    /** @brief The intensity of this directional light. */
    f64 intensity;

    /** @brief The direction of this directional light. */
    Vec3<f64> direction;

};

/** @brief Represents a light illuminating a scene. */
using Light = std::variant<AmbientLight, PointLight, DirectionalLight>;

}

#endif