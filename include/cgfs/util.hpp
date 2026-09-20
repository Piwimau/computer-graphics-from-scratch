#ifndef CGFS_UTIL_HPP
#define CGFS_UTIL_HPP

#include <concepts>
#include <numbers>
#include "cgfs/types.hpp"

namespace cgfs {

/**
 * @brief Converts an angle from degrees to radians.
 *
 * @tparam T The type of the argument.
 * @param[in] degrees The angle in degrees.
 * @return The angle in radians.
 */
template<std::floating_point T>
constexpr T radians(T degrees) noexcept {
    return degrees * std::numbers::pi_v<T> / static_cast<T>(180);
}

/**
 * @brief Converts an angle from radians to degrees.
 *
 * @tparam T The type of the argument.
 * @param[in] radians The angle in radians.
 * @return The angle in degrees.
 */
template<std::floating_point T>
constexpr T degrees(T radians) noexcept {
    return radians * static_cast<T>(180) / std::numbers::pi_v<T>;
}

}

#endif