#ifndef CGFS_RAYTRACER_HPP
#define CGFS_RAYTRACER_HPP

#include <span>
#include "cgfs/camera.hpp"
#include "cgfs/canvas.hpp"
#include "cgfs/light.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/viewport.hpp"

namespace cgfs {

/**
 * @brief Renders a scene using raytracing.
 *
 * @param[in]  spheres  A collection of spheres representing the scene.
 * @param[in]  lights   A collection of lights illuminating the scene.
 * @param[in]  camera   The camera looking at the scene.
 * @param[in]  viewport A viewport acting as a window into the scene.
 * @param[out] canvas   The canvas to render the scene to.
 */
void raytrace(
    std::span<const Sphere> spheres,
    std::span<const Light> lights,
    const Camera& camera,
    const Viewport& viewport,
    Canvas& canvas
) noexcept;

}

#endif