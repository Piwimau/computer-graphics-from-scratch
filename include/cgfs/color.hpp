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
            std::clamp<f32>(r * static_cast<f32>(rhs), 0.0F, 255.0F)
        );
        g = static_cast<u8>(
            std::clamp<f32>(g * static_cast<f32>(rhs), 0.0F, 255.0F)
        );
        b = static_cast<u8>(
            std::clamp<f32>(b * static_cast<f32>(rhs), 0.0F, 255.0F)
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

};

/** @brief The color black. */
static constexpr Color BLACK = { 0, 0, 0 };

}

#endif