#ifndef CGFS_MATERIAL_HPP
#define CGFS_MATERIAL_HPP

#include "cgfs/color.hpp"
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a material of an object. */
struct Material {

    /** @brief The ambient color of this material. */
    Color ambient;

    /** @brief The diffuse color of this material. */
    Color diffuse;

    /** @brief The specular color of this material. */
    Color specular;

    /** @brief The shininess of this material. */
    f64 shininess;

    /** @brief The reflectivity of this material. */
    f64 reflectivity;

};

}

#endif