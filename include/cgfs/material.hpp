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

    /** @brief The reflectance at normal incidence. */
    Color f0;

    /** @brief The metalness of this material. */
    f64 metalness;

    /** @brief The roughness of this material. */
    f64 roughness;

    /** @brief The transparency of this material. */
    f64 transparency;

    /** @brief The index of refraction of this material. */
    f64 ior;

    /** @brief The absorption coefficients of this material. */
    Color absorption;

    /**
     * @brief Creates an opaque material with the specified properties.
     *
     * @warning The behavior is undefined if any component of `albedo` is
     * outside the range `[0.0, 1.0]`, or if `metalness` or `roughness` is
     * outside the range `[0.0, 1.0]`.
     *
     * @param[in] albedo    The base color of the material.
     * @param[in] metalness The metalness of the material.
     * @param[in] roughness The roughness of the material.
     * @return An opaque material with the specified properties.
     */
    static constexpr Material opaque(
        const Color& albedo,
        f64 metalness,
        f64 roughness
    ) noexcept {
        assert((albedo.min() >= 0.0) && (albedo.max() <= 1.0));
        assert((metalness >= 0.0) && (metalness <= 1.0));
        assert((roughness >= 0.0) && (roughness <= 1.0));
        Color f0 = Color::lerp({ 0.04, 0.04, 0.04 }, albedo, metalness);
        return {
            .albedo = albedo,
            .f0 = f0,
            .metalness = metalness,
            .roughness = roughness,
            .transparency = 0.0,
            .ior = 1.0,
            .absorption = { 0.0, 0.0, 0.0 }
        };
    }

    /**
     * @brief Creates a transparent material with the specified properties.
     *
     * @warning The behavior is undefined if any component of `albedo` or
     * `absorption` is outside the range `[0.0, 1.0]`, if `roughness` or
     * `transparency` is outside the range `[0.0, 1.0]`, or if `ior` is not
     * greater than `0.0`.
     *
     * @param[in] albedo       The base color of the material.
     * @param[in] roughness    The roughness of the material.
     * @param[in] transparency The transparency of the material.
     * @param[in] ior          The index of refraction of the material.
     * @param[in] absorption   The absorption coefficients of the material.
     * @return A transparent material with the specified properties.
     */
    static constexpr Material transparent(
        const Color& albedo,
        f64 roughness,
        f64 transparency,
        f64 ior,
        const Color& absorption = { 0.0, 0.0, 0.0 }
    ) noexcept {
        assert((albedo.min() >= 0.0) && (albedo.max() <= 1.0));
        assert((roughness >= 0.0) && (roughness <= 1.0));
        assert((transparency >= 0.0) && (transparency <= 1.0));
        assert(ior > 0.0);
        assert((absorption.min() >= 0.0) && (absorption.max() <= 1.0));
        f64 x = (ior - 1.0) / (ior + 1.0);
        x = x * x;
        return {
            .albedo = albedo,
            .f0 = { x, x, x },
            .metalness = 0.0,
            .roughness = roughness,
            .transparency = transparency,
            .ior = ior,
            .absorption = absorption
        };
    }

    /**
     * @brief Determines if this material is transparent.
     *
     * @return `true` if this material is transparent, otherwise `false`.
     */
    constexpr bool is_transparent() const noexcept {
        return transparency > 0.0;
    }

};

}

#endif