#ifndef CGFS_RAYTRACER_HPP
#define CGFS_RAYTRACER_HPP

#include "cgfs/canvas.hpp"
#include "cgfs/scene.hpp"

namespace cgfs {

/**
 * @brief Renders a scene using raytracing.
 *
 * @warning The behavior is undefined if `samples` is less than or equal to
 * zero.
 *
 * @param[in]  scene   The scene to render.
 * @param[out] canvas  The canvas to render the scene to.
 * @param[in]  samples The number of samples per pixel for anti-aliasing.
 */
void raytrace(const Scene& scene, Canvas& canvas, isize samples = 1);

}

#endif