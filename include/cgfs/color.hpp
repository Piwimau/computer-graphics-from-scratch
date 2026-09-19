#ifndef CGFS_COLOR_HPP
#define CGFS_COLOR_HPP

#include <algorithm>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a color in the RGB color format. */
struct Color {

    /** @brief The red component of this color. */
    f32 r;

    /** @brief The green component of this color. */
    f32 g;

    /** @brief The blue component of this color. */
    f32 b;

    /**
     * @brief Adds a color to this color (component-wise).
     *
     * @param[in] rhs The color to add.
     * @return A reference to this color after addition.
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
     * @return A reference to this color after subtraction.
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
     * @return A reference to this color after multiplication.
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
     * @return A reference to this color after multiplication.
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
     * @return A reference to this color after division.
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
     * @return A reference to this color after division.
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
     * @brief Returns the component-wise minimum of this color and another
     * color.
     *
     * @param[in] rhs The other color.
     * @return The component-wise minimum of this color and another color.
     */
    constexpr Color min(const Color& rhs) const noexcept {
        return {
            std::min(r, rhs.r),
            std::min(g, rhs.g),
            std::min(b, rhs.b)
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
     * @brief Returns the component-wise maximum of this color and another
     * color.
     *
     * @param[in] rhs The other color.
     * @return The component-wise maximum of this color and another color.
     */
    constexpr Color max(const Color& rhs) const noexcept {
        return {
            std::max(r, rhs.r),
            std::max(g, rhs.g),
            std::max(b, rhs.b)
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
            std::clamp(r, min, max),
            std::clamp(g, min, max),
            std::clamp(b, min, max)
        };
    }

    /**
     * @brief Clamps the components of this color to a specified range.
     *
     * @param[in] min The minimum value for each component.
     * @param[in] max The maximum value for each component (inclusive).
     * @return The clamped color.
     */
    constexpr Color clamp(const Color& min, const Color& max) const noexcept {
        return {
            std::clamp(r, min.r, max.r),
            std::clamp(g, min.g, max.g),
            std::clamp(b, min.b, max.b)
        };
    }

    /**
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] a The color at `t = 0.0F`.
     * @param[in] b The color at `t = 1.0F`.
     * @param[in] t The interpolation factor in the range `[0.0F, 1.0F]`.
     * @return The interpolated color.
     */
    static constexpr Color lerp(
        const Color& a,
        const Color& b,
        f32 t
    ) noexcept {
        return a * (1.0F - t) + b * t;
    }

    /**
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] a The color at `t = 0.0F`.
     * @param[in] b The color at `t = 1.0F`.
     * @param[in] t The interpolation factor for each component in the range
     *              `[0.0F, 1.0F]`.
     * @return The interpolated color.
     */
    static constexpr Color lerp(
        const Color& a,
        const Color& b,
        const Color& t
    ) noexcept {
        return a * (Color { 1.0F, 1.0F, 1.0F } - t) + b * t;
    }

};

}

#endif