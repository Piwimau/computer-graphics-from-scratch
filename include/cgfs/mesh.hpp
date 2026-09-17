#ifndef CGFS_MESH_HPP
#define CGFS_MESH_HPP

#include <cassert>
#include <optional>
#include <ranges>
#include <vector>
#include "cgfs/aabb.hpp"
#include "cgfs/material.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a triangle in a mesh. */
struct Triangle {

    /** @brief The first vertex of this triangle. */
    Vec3<f32> v0;

    /** @brief The second vertex of this triangle. */
    Vec3<f32> v1;

    /** @brief The third vertex of this triangle. */
    Vec3<f32> v2;

    /**
     * @brief Returns the normalized surface normal of this triangle.
     *
     * @return The normalized surface normal of this triangle.
     */
    constexpr Vec3<f32> normal() const noexcept {
        return (v1 - v0).cross(v2 - v0).normalize();
    }

};

/** @brief Represents a mesh of triangles. */
struct Mesh {

    /** @brief The vertices that make up this mesh. */
    std::vector<Vec3<f32>> vertices;

    /** @brief The indices forming the triangles of this mesh. */
    std::vector<isize> indices;

    /** @brief An optional axis-aligned bounding box for this mesh. */
    std::optional<Aabb> bounds;

    /** @brief The material of this mesh. */
    Material material;

    /**
     * @brief Creates a new mesh with the specified vertices, indices, and
     * material.
     *
     * @warning The behavior is undefined if `vertices` contains fewer than
     * three elements, or if `indices` is not a multiple of three.
     *
     * @param[in] vertices The vertices that make up the mesh.
     * @param[in] indices  The indices forming the triangles of the mesh.
     * @param[in] material The material of the mesh.
     * @return A new mesh with the specified vertices, indices, and material.
     */
    static constexpr Mesh create(
        std::vector<Vec3<f32>> vertices,
        std::vector<isize> indices,
        const Material& material
    ) noexcept {
        assert(vertices.size() >= 3);
        assert(indices.size() % 3 == 0);
        Aabb bounds = Aabb::create(vertices);
        return {
            .vertices = std::move(vertices),
            .indices = std::move(indices),
            .bounds = bounds,
            .material = material
        };
    }

    /**
     * @brief Returns a view of the triangles in this mesh.
     *
     * @return A view of the triangles in this mesh.
     */
    constexpr auto triangles() const noexcept {
        return indices
            | std::views::chunk(3)
            | std::views::transform(
                [this](auto&& chunk) {
                    return Triangle {
                        vertices[chunk[0]],
                        vertices[chunk[1]],
                        vertices[chunk[2]]
                    };
                }
            );
    }

};

}

#endif