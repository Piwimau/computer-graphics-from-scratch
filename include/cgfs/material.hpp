#ifndef CGFS_MATERIAL_HPP
#define CGFS_MATERIAL_HPP

#include <cassert>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a material of an object. */
struct Material {

    /** @brief The base color of this material. */
    Color albedo;

    /** @brief The reflectance color at normal incidence. */
    Color f0;

    /** @brief The metalness of this material. */
    f64 metalness;

    /** @brief The roughness of this material. */
    f64 roughness;

    /**
     * @brief Creates a new material with the specified properties.
     *
     * @warning The behavior is undefined if `metalness` or `roughness` are
     * outside the range `[0.0, 1.0]`.
     *
     * @param[in] albedo    The base color of the material.
     * @param[in] metalness The metalness of the material.
     * @param[in] roughness The roughness of the material.
     * @return A new material with the specified properties.
     */
    static constexpr Material create(
        const Color& albedo,
        f64 metalness,
        f64 roughness
    ) noexcept {
        assert((metalness >= 0.0) && (metalness <= 1.0));
        assert((roughness >= 0.0) && (roughness <= 1.0));
        Color f0 = Color::lerp({ 0.04, 0.04, 0.04 }, albedo, metalness);
        return { albedo, f0, metalness, roughness };
    }

};

}

#endif