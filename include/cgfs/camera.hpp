#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents a camera. */
struct Camera {

    /** @brief The position of this camera. */
    Vec3<f64> position;

    /** @brief The direction this camera is facing. */
    Vec3<f64> orientation;

};

}

#endif