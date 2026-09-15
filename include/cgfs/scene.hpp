#ifndef CGFS_SCENE_HPP
#define CGFS_SCENE_HPP

#include <variant>
#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/light.hpp"
#include "cgfs/mesh.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/viewport.hpp"

namespace cgfs {

/** @brief Represents an object in a scene. */
using Object = std::variant<Sphere, Mesh>;

/** @brief Represents a scene to render. */
struct Scene {

    /** @brief The objects that make up this scene. */
    std::vector<Object> objects;

    /** @brief The ambient light illuminating this scene. */
    AmbientLight ambientLight;

    /** @brief The point lights illuminating this scene. */
    std::vector<PointLight> pointLights;

    /** @brief The directional lights illuminating this scene. */
    std::vector<DirectionalLight> directionalLights;

    /** @brief The viewport through which this scene is rendered. */
    Viewport viewport;

    /** @brief The camera through which this scene is viewed. */
    Camera camera;

};

}

#endif