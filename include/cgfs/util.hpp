#ifndef CGFS_UTIL_HPP
#define CGFS_UTIL_HPP

#include <concepts>
#include <limits>
#include <numbers>

namespace cgfs {

/**
 * @brief The value of pi.
 *
 * @tparam T The desired type.
 */
template<std::floating_point T>
constexpr inline T PI = std::numbers::pi_v<T>;

/**
 * @brief The value of infinity.
 *
 * @tparam The desired type.
 */
template<std::floating_point T>
constexpr inline T INF = std::numeric_limits<T>::infinity();

/**
 * @brief Converts an angle from degrees to radians.
 *
 * @tparam T The type of the argument.
 * @param[in] degrees The angle in degrees.
 * @return The angle in radians.
 */
template<std::floating_point T>
constexpr T radians(T degrees) noexcept {
    return degrees * PI<T> / static_cast<T>(180);
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
    return radians * static_cast<T>(180) / PI<T>;
}

}

#endif