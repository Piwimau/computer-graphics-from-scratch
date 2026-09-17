#ifndef CGFS_VIEWPORT_HPP
#define CGFS_VIEWPORT_HPP

#include <cassert>
#include <cmath>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a viewport which acts as window into a scene. */
struct Viewport {

    /** @brief The width of this viewport. */
    f32 width;

    /** @brief The height of this viewport. */
    f32 height;

    /** @brief The distance to the camera. */
    f32 distance;

    /**
     * @brief Creates a viewport with the specified parameters.
     *
     * @warning The behavior is undefined if `fov`, `aspectRatio`, or `distance`
     * is less than or equal to zero.
     *
     * @param[in] fov         The vertical field of view (in radians).
     * @param[in] aspectRatio The aspect ratio (width / height).
     * @param[in] distance    The distance to the camera.
     * @return A viewport with the specified parameters.
     */
    static constexpr Viewport create(
        f32 fov,
        f32 aspectRatio,
        f32 distance = 1.0F
    ) noexcept {
        assert(fov > 0.0F);
        assert(aspectRatio > 0.0F);
        assert(distance > 0.0F);
        f32 height = 2.0F * distance * std::tan(fov / 2.0F);
        f32 width = height * aspectRatio;
        return { width, height, distance };
    }

};

}

#endif