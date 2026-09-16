#ifndef CGFS_VIEWPORT_HPP
#define CGFS_VIEWPORT_HPP

#include <cassert>
#include <cmath>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a viewport which acts as window into a scene. */
struct Viewport {

    /** @brief The width of this viewport. */
    f64 width;

    /** @brief The height of this viewport. */
    f64 height;

    /** @brief The distance to the camera. */
    f64 distance;

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
        f64 fov,
        f64 aspectRatio,
        f64 distance = 1.0
    ) noexcept {
        assert(fov > 0.0);
        assert(aspectRatio > 0.0);
        assert(distance > 0.0);
        f64 height = 2.0 * distance * std::tan(fov / 2.0);
        f64 width = height * aspectRatio;
        return { width, height, distance };
    }

};

}

#endif