#ifndef CGFS_COLOR_HPP
#define CGFS_COLOR_HPP

#include <algorithm>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a color in the RGB color format. */
struct Color {

    /** @brief The red component of this color. */
    f64 r;

    /** @brief The green component of this color. */
    f64 g;

    /** @brief The blue component of this color. */
    f64 b;

    /**
     * @brief Adds a color to this color (component-wise).
     *
     * @param[in] rhs The color to add.
     * @return A reference to this color after addition.
     */
    constexpr Color& operator+=(const Color& rhs) noexcept {
        r = std::clamp(r + rhs.r, 0.0, 1.0);
        g = std::clamp(g + rhs.g, 0.0, 1.0);
        b = std::clamp(b + rhs.b, 0.0, 1.0);
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
        r = std::clamp(r - rhs.r, 0.0, 1.0);
        g = std::clamp(g - rhs.g, 0.0, 1.0);
        b = std::clamp(b - rhs.b, 0.0, 1.0);
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
     * @param rhs The scalar to multiply by.
     * @return A reference to this color after multiplication.
     */
    constexpr Color& operator*=(f64 rhs) noexcept {
        r = std::clamp(r * rhs, 0.0, 1.0);
        g = std::clamp(g * rhs, 0.0, 1.0);
        b = std::clamp(b * rhs, 0.0, 1.0);
        return *this;
    }

    /**
     * @brief Multiplies a color by a scalar (component-wise).
     *
     * @param[in] lhs The color to scale.
     * @param[in] rhs The scalar to multiply by.
     * @return The result of multiplying the color by the scalar.
     */
    friend constexpr Color operator*(Color lhs, f64 rhs) noexcept {
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
    friend constexpr Color operator*(f64 lhs, Color rhs) noexcept {
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
        r = std::clamp(r * rhs.r, 0.0, 1.0);
        g = std::clamp(g * rhs.g, 0.0, 1.0);
        b = std::clamp(b * rhs.b, 0.0, 1.0);
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
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] a The color at `t = 0.0`.
     * @param[in] b The color at `t = 1.0`.
     * @param[in] t The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated color.
     */
    static constexpr Color lerp(
        const Color& a,
        const Color& b,
        f64 t
    ) noexcept {
        return a * (1.0 - t) + b * t;
    }

};

}

#endif