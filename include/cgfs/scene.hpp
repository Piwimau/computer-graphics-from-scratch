#ifndef CGFS_SCENE_HPP
#define CGFS_SCENE_HPP

#include <optional>
#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/light.hpp"
#include "cgfs/material.hpp"
#include "cgfs/sky.hpp"
#include "cgfs/sphere.hpp"

namespace cgfs {

/** @brief A scene to be rendered. */
struct Scene {

    /** @brief The materials used in this scene. */
    std::vector<Material> materials;

    /** @brief The spheres positioned in this scene. */
    std::vector<Sphere> spheres;

    /** @brief The lights illuminating this scene. */
    std::vector<Light> lights;

    /** @brief The sky of this scene (if any). */
    std::optional<Sky> sky;

    /** @brief The camera through which this scene is viewed. */
    Camera camera;

};

}

#endif