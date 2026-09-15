#ifndef CGFS_RAYTRACER_HPP
#define CGFS_RAYTRACER_HPP

#include "cgfs/canvas.hpp"
#include "cgfs/scene.hpp"

namespace cgfs {

/**
 * @brief Renders a scene using raytracing.
 *
 * @param[in]  scene  The scene to render.
 * @param[out] canvas The canvas to render the scene to.
 */
void raytrace(const Scene& scene, Canvas& canvas);

}

#endif