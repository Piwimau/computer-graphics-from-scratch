#ifndef CGFS_COLOR_HPP
#define CGFS_COLOR_HPP

#include <algorithm>
#include <cmath>
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"

namespace cgfs {

/** @brief A color in the RGB color format. */
struct Color {

    /** @brief The red component. */
    f32 r;

    /** @brief The green component. */
    f32 g;

    /** @brief The blue component. */
    f32 b;

    /**
     * @brief Constructs a color with all components set to a specified value.
     *
     * @param[in] value The value for all components.
     * @return A color with all components set to the specified value.
     */
    static constexpr Color splat(f32 value) noexcept {
        return { .r = value, .g = value, .b = value };
    }

    /**
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] lhs The color at `t = 0.0F`.
     * @param[in] rhs The color at `t = 1.0F`.
     * @param[in] t   The interpolation factor in the range `[0.0F, 1.0F]`.
     * @return The interpolated color.
     */
    static constexpr Color lerp(
        const Color& lhs,
        const Color& rhs,
        f32 t
    ) noexcept {
        return lhs * (1.0F - t) + rhs * t;
    }

    /**
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] lhs The color at `t = 0.0F`.
     * @param[in] rhs The color at `t = 1.0F`.
     * @param[in] t   The interpolation factors for the components in the range
     *                `[0.0F, 1.0F]`.
     * @return The interpolated color.
     */
    static constexpr Color lerp(
        const Color& lhs,
        const Color& rhs,
        const Color& t
    ) noexcept {
        return lhs * (Color::splat(1.0F) - t) + rhs * t;
    }

    /**
     * @brief Adds a color to this color (component-wise).
     *
     * @param[in] rhs The color to add.
     * @return A reference to this color.
     */
    constexpr Color& operator+=(const Color& rhs) noexcept {
        r = r + rhs.r;
        g = g + rhs.g;
        b = b + rhs.b;
        return *this;
    }

    /**
     * @brief Adds two colors (component-wise).
     *
     * @param[in] lhs The color to add to.
     * @param[in] rhs The color to add.
     * @return The result of adding the two colors.
     */
    friend constexpr Color operator+(Color lhs, const Color& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }

    /**
     * @brief Subtracts a color from this color (component-wise).
     *
     * @param[in] rhs The color to subtract.
     * @return A reference to this color.
     */
    constexpr Color& operator-=(const Color& rhs) noexcept {
        r = r - rhs.r;
        g = g - rhs.g;
        b = b - rhs.b;
        return *this;
    }

    /**
     * @brief Subtracts a color from another (component-wise).
     *
     * @param[in] lhs The color to subtract from.
     * @param[in] rhs The color to subtract.
     * @return The result of subtracting the second color from the first.
     */
    friend constexpr Color operator-(Color lhs, const Color& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies this color by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this color.
     */
    constexpr Color& operator*=(f32 rhs) noexcept {
        r = r * rhs;
        g = g * rhs;
        b = b * rhs;
        return *this;
    }

    /**
     * @brief Multiplies a color by a scalar (component-wise).
     *
     * @param[in] lhs The color to scale.
     * @param[in] rhs The scalar to multiply by.
     * @return The result of multiplying the color by the scalar.
     */
    friend constexpr Color operator*(Color lhs, f32 rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies a color by a scalar (component-wise).
     *
     * @param[in] lhs The scalar to multiply by.
     * @param[in] rhs The color to scale.
     * @return The result of multiplying the color by the scalar.
     */
    friend constexpr Color operator*(f32 lhs, Color rhs) noexcept {
        rhs *= lhs;
        return rhs;
    }

    /**
     * @brief Multiplies this color by another color (component-wise).
     *
     * @param[in] rhs The color to multiply by.
     * @return A reference to this color.
     */
    constexpr Color& operator*=(const Color& rhs) noexcept {
        r = r * rhs.r;
        g = g * rhs.g;
        b = b * rhs.b;
        return *this;
    }

    /**
     * @brief Multiplies two colors (component-wise).
     *
     * @param[in] lhs The first color.
     * @param[in] rhs The second color.
     * @return The result of multiplying the two colors.
     */
    friend constexpr Color operator*(Color lhs, const Color& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Divides this color by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to divide by.
     * @return A reference to this color.
     */
    constexpr Color& operator/=(f32 rhs) noexcept {
        r = r / rhs;
        g = g / rhs;
        b = b / rhs;
        return *this;
    }

    /**
     * @brief Divides a color by a scalar (component-wise).
     *
     * @param[in] lhs The color to scale.
     * @param[in] rhs The scalar to divide by.
     * @return The result of dividing the color by the scalar.
     */
    friend constexpr Color operator/(Color lhs, f32 rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    /**
     * @brief Divides this color by another color (component-wise).
     *
     * @param[in] rhs The color to divide by.
     * @return A reference to this color.
     */
    constexpr Color& operator/=(const Color& rhs) noexcept {
        r = r / rhs.r;
        g = g / rhs.g;
        b = b / rhs.b;
        return *this;
    }

    /**
     * @brief Divides two colors (component-wise).
     *
     * @param[in] lhs The color to divide.
     * @param[in] rhs The color to divide by.
     * @return The result of dividing the first color by the second.
     */
    friend constexpr Color operator/(Color lhs, const Color& rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    /**
     * @brief Returns the minimum component of this color.
     *
     * @return The minimum component of this color.
     */
    constexpr f32 min() const noexcept {
        return std::min({ r, g, b });
    }

    /**
     * @brief Returns the minimum of this color and another color
     * (component-wise).
     *
     * @param[in] rhs The other color.
     * @return The minimum of this color and another color (component-wise).
     */
    constexpr Color min(const Color& rhs) const noexcept {
        return {
            .r = std::min(r, rhs.r),
            .g = std::min(g, rhs.g),
            .b = std::min(b, rhs.b)
        };
    }

    /**
     * @brief Returns the maximum component of this color.
     *
     * @return The maximum component of this color.
     */
    constexpr f32 max() const noexcept {
        return std::max({ r, g, b });
    }

    /**
     * @brief Returns the maximum of this color and another color
     * (component-wise).
     *
     * @param[in] rhs The other color.
     * @return The maximum of this color and another color (component-wise).
     */
    constexpr Color max(const Color& rhs) const noexcept {
        return {
            .r = std::max(r, rhs.r),
            .g = std::max(g, rhs.g),
            .b = std::max(b, rhs.b)
        };
    }

    /**
     * @brief Clamps the components of this color to a specified range.
     *
     * @param[in] min The minimum value.
     * @param[in] max The maximum value (inclusive).
     * @return The clamped color.
     */
    constexpr Color clamp(f32 min, f32 max) const noexcept {
        return {
            .r = std::clamp(r, min, max),
            .g = std::clamp(g, min, max),
            .b = std::clamp(b, min, max)
        };
    }

    /**
     * @brief Clamps the components of this color to a specified range.
     *
     * @param[in] min The minimum values for the components.
     * @param[in] max The maximum values for the components (inclusive).
     * @return The clamped color.
     */
    constexpr Color clamp(const Color& min, const Color& max) const noexcept {
        return {
            .r = std::clamp(r, min.r, max.r),
            .g = std::clamp(g, min.g, max.g),
            .b = std::clamp(b, min.b, max.b)
        };
    }

    /**
     * @brief Applies the Reinhard tone mapping operator to this color.
     *
     * @return The tone-mapped color.
     */
    constexpr Color tone_map_reinhard() const noexcept {
        f32 luminance = 0.2126F * r + 0.7152F * g + 0.0722F * b;
        return (*this / (1.0F + luminance)).clamp(0.0F, 1.0F);
    }

    /**
     * @brief Applies the ACES tone mapping operator to this color.
     *
     * @return The tone-mapped color.
     */
    constexpr Color tone_map_aces() const noexcept {
        Color v = {
            .r = 0.59719F * r + 0.35458F * g + 0.04823F * b,
            .g = 0.07600F * r + 0.90834F * g + 0.01566F * b,
            .b = 0.02840F * r + 0.13383F * g + 0.83777F * b
        };
        auto f = [](f32 c) {
            f32 a = c * (c + 0.0245786F) - 0.000090537F;
            f32 b = c * (0.983729F * c + 0.4329510F) + 0.238081F;
            return a / b;
        };
        v = { .r = f(v.r), .g = f(v.g), .b = f(v.b) };
        v = {
            .r = 1.60475F * v.r - 0.53108F * v.g - 0.07367F * v.b,
            .g = -0.10208F * v.r + 1.10813F * v.g - 0.00605F * v.b,
            .b = -0.00327F * v.r - 0.07276F * v.g + 1.07602F * v.b
        };
        return v.clamp(0.0F, 1.0F);
    }

    /**
     * @brief Applies the Khronos PBR Neutral tone mapping operator to this
     * color.
     *
     * @return The tone-mapped color.
     */
    constexpr Color tone_map_khronos_pbr_neutral() const noexcept {
        constexpr f32 START_COMPRESSION = 0.8F - 0.04F;
        constexpr f32 DESATURATION = 0.15F;
        f32 x = min();
        f32 offset = (x < 0.08F) ? x - 6.25F * x * x : 0.04F;
        Color c = *this - Color::splat(offset);
        f32 peak = c.max();
        if (peak < START_COMPRESSION) {
            return c;
        }
        f32 d = 1.0F - START_COMPRESSION;
        f32 newPeak = 1.0F - d * d / (peak + d - START_COMPRESSION);
        c *= newPeak / peak;
        f32 t = 1.0F - 1.0F / (DESATURATION * (peak - newPeak) + 1.0F);
        return c + (Color::splat(newPeak) - c) * t;
    }

    /**
     * @brief Converts this color from linear space to sRGB space.
     *
     * @return The color in sRGB space.
     *
     * @note This function assumes that this color is in linear space.
     */
    constexpr Color to_srgb() const noexcept {
        auto f = [](f32 c) {
            return (c <= 0.0031308F)
                ? 12.92F * c
                : 1.055F * std::pow(c, 1.0F / 2.4F) - 0.055F;
        };
        return { .r = f(r), .g = f(g), .b = f(b) };
    }

    /**
     * @brief Converts this color from sRGB space to linear space.
     *
     * @return The color in linear space.
     *
     * @note This function assumes that this color is in sRGB space.
     */
    constexpr Color to_linear() const noexcept {
        auto f = [](f32 c) {
            return (c <= 0.04045F)
                ? c / 12.92F
                : std::pow((c + 0.055F) / 1.055F, 2.4F);
        };
        return { .r = f(r), .g = f(g), .b = f(b) };
    }

};

}

#endif