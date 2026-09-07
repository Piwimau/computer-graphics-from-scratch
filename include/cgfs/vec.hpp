#ifndef CGFS_VEC_HPP
#define CGFS_VEC_HPP

#include <concepts>

namespace cgfs {

/**
 * @brief Represents a two-dimensional vector.
 *
 * @tparam T The type of the vector's components.
 */
template<typename T>
requires (std::integral<T> || std::floating_point<T>)
struct Vec2 {

    /** @brief The x-component of this vector. */
    T x;

    /** @brief The y-component of this vector. */
    T y;

};

/**
 * @brief Represents a three-dimensional vector.
 *
 * @tparam T The type of the vector's components.
 */
template<typename T>
requires (std::integral<T> || std::floating_point<T>)
struct Vec3 {

    /** @brief The x-component of this vector. */
    T x;

    /** @brief The y-component of this vector. */
    T y;

    /** @brief The z-component of this vector. */
    T z;

};

}

#endif