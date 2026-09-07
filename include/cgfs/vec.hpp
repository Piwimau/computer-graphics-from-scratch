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

    /**
     * @brief Adds a vector to this vector (component-wise).
     *
     * @param[in] rhs The vector to add.
     * @return A reference to this vector after the addition.
     */
    constexpr Vec2<T>& operator+=(const Vec2<T>& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /**
     * @brief Adds two vectors (component-wise).
     *
     * @param[in] lhs The first vector.
     * @param[in] rhs The second vector.
     * @return The result of adding the two vectors together.
     */
    friend constexpr Vec2<T> operator+(
        Vec2<T> lhs,
        const Vec2<T>& rhs
    ) noexcept {
        lhs += rhs;
        return lhs;
    }

    /**
     * @brief Subtracts a vector from this vector (component-wise).
     *
     * @param[in] rhs The vector to subtract.
     * @return A reference to this vector after the subtraction.
     */
    constexpr Vec2<T>& operator-=(const Vec2<T>& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    /**
     * @brief Subtracts two vectors (component-wise).
     *
     * @param[in] lhs The first vector.
     * @param[in] rhs The second vector.
     * @return The result of subtracting the second vector from the first.
     */
    friend constexpr Vec2<T> operator-(
        Vec2<T> lhs,
        const Vec2<T>& rhs
    ) noexcept {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Computes the dot product of this vector and another vector.
     *
     * @param[in] rhs The other vector.
     * @return The dot product of this vector and the other vector.
     */
    constexpr T dot(const Vec2<T>& rhs) const noexcept {
        return x * rhs.x + y * rhs.y;
    }

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

    /**
     * @brief Adds a vector to this vector (component-wise).
     *
     * @param[in] rhs The vector to add.
     * @return A reference to this vector after the addition.
     */
    constexpr Vec3<T>& operator+=(const Vec3<T>& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    /**
     * @brief Adds two vectors (component-wise).
     *
     * @param[in] lhs The first vector.
     * @param[in] rhs The second vector.
     * @return The result of adding the two vectors together.
     */
    friend constexpr Vec3<T> operator+(
        Vec3<T> lhs,
        const Vec3<T>& rhs
    ) noexcept {
        lhs += rhs;
        return lhs;
    }

    /**
     * @brief Subtracts a vector from this vector (component-wise).
     *
     * @param[in] rhs The vector to subtract.
     * @return A reference to this vector after the subtraction.
     */
    constexpr Vec3<T>& operator-=(const Vec3<T>& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    /**
     * @brief Subtracts two vectors (component-wise).
     *
     * @param[in] lhs The first vector.
     * @param[in] rhs The second vector.
     * @return The result of subtracting the second vector from the first.
     */
    friend constexpr Vec3<T> operator-(
        Vec3<T> lhs,
        const Vec3<T>& rhs
    ) noexcept {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Computes the dot product of this vector and another vector.
     *
     * @param[in] rhs The other vector.
     * @return The dot product of this vector and the other vector.
     */
    constexpr T dot(const Vec3<T>& rhs) const noexcept {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

};

}

#endif