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
    f32 metalness;

    /** @brief The roughness of this material. */
    f32 roughness;

    /** @brief The transparency of this material. */
    f32 transparency;

    /** @brief The index of refraction of this material. */
    f32 ior;

    /** @brief The absorption coefficients of this material. */
    Color absorption;

    /** @brief The emission of this material. */
    Color emission;

    /**
     * @brief Creates an opaque material with the specified properties.
     *
     * @warning The behavior is undefined if any component of `albedo` is
     * outside the range `[0.0F, 1.0F]`, or if `metalness` or `roughness` is
     * outside the range `[0.0F, 1.0F]`.
     *
     * @param[in] albedo    The base color of the material.
     * @param[in] metalness The metalness of the material.
     * @param[in] roughness The roughness of the material.
     * @param[in] emission  The emission of the material.
     * @return An opaque material with the specified properties.
     */
    static constexpr Material opaque(
        const Color& albedo,
        f32 metalness,
        f32 roughness,
        const Color& emission = { 0.0F, 0.0F, 0.0F }
    ) noexcept {
        assert((albedo.min() >= 0.0F) && (albedo.max() <= 1.0F));
        assert((metalness >= 0.0F) && (metalness <= 1.0F));
        assert((roughness >= 0.0F) && (roughness <= 1.0F));
        Color f0 = Color::lerp({ 0.04F, 0.04F, 0.04F }, albedo, metalness);
        return {
            .albedo = albedo,
            .f0 = f0,
            .metalness = metalness,
            .roughness = roughness,
            .transparency = 0.0F,
            .ior = 1.0F,
            .absorption = { 0.0F, 0.0F, 0.0F },
            .emission = emission
        };
    }

    /**
     * @brief Creates a transparent material with the specified properties.
     *
     * @warning The behavior is undefined if any component of `albedo` or
     * `absorption` is outside the range `[0.0F, 1.0F]`, if `roughness` or
     * `transparency` is outside the range `[0.0F, 1.0F]`, or if `ior` is not
     * greater than `0.0F`.
     *
     * @param[in] albedo       The base color of the material.
     * @param[in] roughness    The roughness of the material.
     * @param[in] transparency The transparency of the material.
     * @param[in] ior          The index of refraction of the material.
     * @param[in] absorption   The absorption coefficients of the material.
     * @param[in] emission     The emission of the material.
     * @return A transparent material with the specified properties.
     */
    static constexpr Material transparent(
        const Color& albedo,
        f32 roughness,
        f32 transparency,
        f32 ior,
        const Color& absorption = { 0.0F, 0.0F, 0.0F },
        const Color& emission = { 0.0F, 0.0F, 0.0F }
    ) noexcept {
        assert((albedo.min() >= 0.0F) && (albedo.max() <= 1.0F));
        assert((roughness >= 0.0F) && (roughness <= 1.0F));
        assert((transparency >= 0.0F) && (transparency <= 1.0F));
        assert(ior > 0.0F);
        assert((absorption.min() >= 0.0F) && (absorption.max() <= 1.0F));
        f32 x = (ior - 1.0F) / (ior + 1.0F);
        x = x * x;
        return {
            .albedo = albedo,
            .f0 = { x, x, x },
            .metalness = 0.0F,
            .roughness = roughness,
            .transparency = transparency,
            .ior = ior,
            .absorption = absorption,
            .emission = emission
        };
    }

    /**
     * @brief Determines if this material is opaque.
     *
     * @return `true` if this material is opaque, otherwise `false`.
     */
    constexpr bool is_opaque() const noexcept {
        return transparency == 0.0F;
    }

    /**
     * @brief Determines if this material is transparent.
     *
     * @return `true` if this material is transparent, otherwise `false`.
     */
    constexpr bool is_transparent() const noexcept {
        return transparency > 0.0F;
    }

    /**
     * @brief Determines if this material emits light.
     *
     * @return `true` if this material emits light, otherwise `false`.
     */
    constexpr bool is_emissive() const noexcept {
        return emission.max() > 0.0F;
    }

};

}

#endif