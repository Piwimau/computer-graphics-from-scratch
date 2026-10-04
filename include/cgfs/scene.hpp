#ifndef CGFS_SCENE_HPP
#define CGFS_SCENE_HPP

#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/light.hpp"
#include "cgfs/sphere.hpp"

namespace cgfs {

/** @brief A scene containing spheres, lights, and a camera. */
struct Scene {

    /** @brief The spheres positioned in this scene. */
    std::vector<Sphere> spheres;

    /** @brief The lights illuminating this scene. */
    std::vector<Light> lights;

    /** @brief The camera through which this scene is viewed. */
    Camera camera;

};

}

#endif