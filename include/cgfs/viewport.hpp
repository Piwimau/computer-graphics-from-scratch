#ifndef CGFS_VIEWPORT_HPP
#define CGFS_VIEWPORT_HPP

#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a viewport which acts as window into a scene. */
struct Viewport {

    /** @brief The width of this viewport. */
    f32 width;

    /** @brief The height of this viewport. */
    f32 height;

    /** @brief The distance of this viewport to the camera. */
    f32 distance;

};

}

#endif