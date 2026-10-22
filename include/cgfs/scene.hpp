#ifndef CGFS_SCENE_HPP
#define CGFS_SCENE_HPP

#include <optional>
#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/light.hpp"
#include "cgfs/material.hpp"
#include "cgfs/object.hpp"
#include "cgfs/sky.hpp"

namespace cgfs {

/** @brief A scene to be rendered. */
struct Scene {

    /** @brief The materials used in this scene. */
    std::vector<Material> materials;

    /**
     * @brief The objects positioned in this scene.
     *
     * @note This vector must be ordered such that opaque objects appear before
     * transparent objects.
     */
    std::vector<Object> objects;

    /** @brief The lights illuminating this scene. */
    std::vector<Light> lights;

    /** @brief The sky of this scene (if any). */
    std::optional<Sky> sky;

    /** @brief The camera through which this scene is viewed. */
    Camera camera;

};

}

#endif