#ifndef CGFS_VIEWPORT_HPP
#define CGFS_VIEWPORT_HPP

#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a viewport which acts as window into a scene. */
struct Viewport {

    /** @brief The width of this viewport. */
    f64 width;

    /** @brief The height of this viewport. */
    f64 height;

    /** @brief The distance of this viewport to the camera. */
    f64 distance;

};

}

#endif