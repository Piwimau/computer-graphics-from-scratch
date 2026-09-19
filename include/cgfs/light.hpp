#ifndef CGFS_LIGHT_HPP
#define CGFS_LIGHT_HPP

#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a point light radiating from a fixed position. */
struct PointLight {

    /** @brief The color of this light. */
    Color color;

    /** @brief The intensity of this light. */
    f32 intensity;

    /** @brief The position of this light. */
    Vec3<f32> position;

    /** @brief The radius of this light. */
    f32 radius;

    /** @brief The constant attenuation factor of this light. */
    f32 kc;

    /** @brief The linear attenuation factor of this light. */
    f32 kl;

    /** @brief The quadratic attenuation factor of this light. */
    f32 kq;

};

/** @brief Represents a spot light radiating from a position in a cone. */
struct SpotLight {

    /** @brief The color of this light. */
    Color color;

    /** @brief The intensity of this light. */
    f32 intensity;

    /** @brief The position of this light. */
    Vec3<f32> position;

    /** @brief The direction this light points towards. */
    Vec3<f32> direction;

    /** @brief The radius of this light. */
    f32 radius;

    /** @brief The cosine of the inner cone angle (in radians). */
    f32 innerCutoff;

    /** @brief The cosine of the outer cone angle (in radians). */
    f32 outerCutoff;

    /** @brief The constant attenuation factor of this light. */
    f32 kc;

    /** @brief The linear attenuation factor of this light. */
    f32 kl;

    /** @brief The quadratic attenuation factor of this light. */
    f32 kq;

};

/** @brief Represents a directional light radiating in a fixed direction. */
struct DirectionalLight {

    /** @brief The color of this light. */
    Color color;

    /** @brief The intensity of this light. */
    f32 intensity;

    /** @brief The direction of this light. */
    Vec3<f32> direction;

    /** @brief The radius of this light. */
    f32 radius;

};

}

#endif