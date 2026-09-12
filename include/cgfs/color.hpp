#ifndef CGFS_COLOR_HPP
#define CGFS_COLOR_HPP

#include <algorithm>
#include <concepts>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a color in the RGB888 color format. */
struct Color {

    /** @brief The red component of this color. */
    u8 r;

    /** @brief The green component of this color. */
    u8 g;

    /** @brief The blue component of this color. */
    u8 b;

    /**
     * @brief Adds another color to this color, clamping the components to the
     * range `[0, 255]`.
     *
     * @param[in] rhs The color to add.
     * @return A reference to this color after the addition.
     */
    constexpr Color& operator+=(const Color& rhs) noexcept {
        r = static_cast<u8>(std::clamp<isize>(r + rhs.r, 0, 255));
        g = static_cast<u8>(std::clamp<isize>(g + rhs.g, 0, 255));
        b = static_cast<u8>(std::clamp<isize>(b + rhs.b, 0, 255));
        return *this;
    }

    /**
     * @brief Adds two colors together, clamping the components to the range
     * `[0, 255]`.
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
     * @brief Subtracts another color from this color, clamping the components
     * to the range `[0, 255]`.
     *
     * @param[in] rhs The color to subtract.
     * @return A reference to this color after the subtraction.
     */
    constexpr Color& operator-=(const Color& rhs) noexcept {
        r = static_cast<u8>(std::clamp<isize>(r - rhs.r, 0, 255));
        g = static_cast<u8>(std::clamp<isize>(g - rhs.g, 0, 255));
        b = static_cast<u8>(std::clamp<isize>(b - rhs.b, 0, 255));
        return *this;
    }

    /**
     * @brief Subtracts two colors, clamping the components to the range `[0,
     * 255]`.
     *
     * @param[in] lhs The color to subtract from.
     * @param[in] rhs The color to subtract.
     * @return The result of subtracting the second color from the first color.
     */
    friend constexpr Color operator-(Color lhs, const Color& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies this color by a scalar, clamping the components to the
     * range `[0, 255]`.
     *
     * @tparam T The type of the scalar.
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this color after the multiplication.
     */
    template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
    constexpr Color& operator*=(T rhs) noexcept {
        r = static_cast<u8>(
            std::clamp<f64>(r * static_cast<f64>(rhs), 0.0, 255.0)
        );
        g = static_cast<u8>(
            std::clamp<f64>(g * static_cast<f64>(rhs), 0.0, 255.0)
        );
        b = static_cast<u8>(
            std::clamp<f64>(b * static_cast<f64>(rhs), 0.0, 255.0)
        );
        return *this;
    }

    /**
     * @brief Multiplies a color by a scalar, clamping its components to the
     * range `[0, 255]`.
     *
     * @tparam T The type of the scalar.
     * @param[in] lhs The color to scale.
     * @param[in] rhs The scalar to multiply by.
     * @return The result of multiplying the color by the scalar.
     */
    template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
    friend constexpr Color operator*(Color lhs, T rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies a color by a scalar, clamping its components to the
     * range `[0, 255]`.
     *
     * @tparam T The type of the scalar.
     * @param[in] lhs The scalar to multiply by.
     * @param[in] rhs The color to scale.
     * @return The result of multiplying the color by the scalar.
     */
    template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
    friend constexpr Color operator*(T lhs, Color rhs) noexcept {
        rhs *= lhs;
        return rhs;
    }

    /**
     * @brief Linearly interpolates between two colors.
     *
     * @param[in] a The color at `t = 0`.
     * @param[in] b The color at `t = 1`.
     * @param[in] t The interpolation factor in the range `[0, 1]`.
     * @return The interpolated color.
     */
    template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
    static constexpr Color lerp(const Color& a, const Color& b, T t) noexcept {
        return a * (static_cast<T>(1) - t) + b * t;
    }

};

/** @brief The color black. */
static constexpr Color BLACK = { 0, 0, 0 };

}

#endif