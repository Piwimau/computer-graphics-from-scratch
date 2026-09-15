#ifndef CGFS_MESH_HPP
#define CGFS_MESH_HPP

#include <vector>
#include "cgfs/material.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a mesh. */
struct Mesh {

    /** @brief The vertices that make up this mesh. */
    std::vector<Vec3<f64>> vertices;

    /** @brief The vertex indices forming the triangles of this mesh. */
    std::vector<isize> indices;

    /** @brief The material of this mesh. */
    Material material;

};

}

#endif