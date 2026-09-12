#ifndef CGFS_VECTOR_HPP
#define CGFS_VECTOR_HPP

#include <cmath>
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
     * @brief Negates this vector (component-wise).
     *
     * @return The negation of this vector.
     */
    constexpr Vec2<T> operator-() const noexcept {
        return { -x, -y };
    }

    /**
     * @brief Multiplies this vector by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this vector after the multiplication.
     */
    constexpr Vec2<T>& operator*=(T rhs) noexcept {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    /**
     * @brief Multiplies a vector by a scalar (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The scalar to multiply by.
     * @return The result of multiplying the vector by the scalar.
     */
    friend constexpr Vec2<T> operator*(Vec2<T> lhs, T rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies a vector by a scalar (component-wise).
     *
     * @param[in] lhs The scalar to multiply by.
     * @param[in] rhs The vector to scale.
     * @return The result of multiplying the vector by the scalar.
     */
    friend constexpr Vec2<T> operator*(T lhs, Vec2<T> rhs) noexcept {
        rhs *= lhs;
        return rhs;
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

    /**
     * @brief Computes the euclidean norm (or magnitude) of this vector.
     *
     * @return The euclidean norm (or magnitude) of this vector.
     */
    constexpr T norm() const noexcept {
        return std::sqrt(this->dot(*this));
    }

    /**
     * @brief Computes the normalized (unit) vector of this vector.
     *
     * @return The normalized (unit) vector of this vector.
     */
    constexpr Vec2<T> normalize() const noexcept {
        T n = norm();
        return { x / n, y / n };
    }

    /**
     * @brief Computes the reflection of this vector around a surface normal.
     *
     * @param[in] normal The surface normal to reflect around.
     * @return The reflected vector.
     */
    constexpr Vec2<T> reflect(const Vec2<T>& normal) const noexcept {
        return static_cast<T>(2) * normal * normal.dot(*this) - *this;
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
     * @brief Negates this vector (component-wise).
     *
     * @return The negation of this vector.
     */
    constexpr Vec3<T> operator-() const noexcept {
        return { -x, -y, -z };
    }

    /**
     * @brief Multiplies this vector by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this vector after the multiplication.
     */
    constexpr Vec3<T>& operator*=(T rhs) noexcept {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    /**
     * @brief Multiplies a vector by a scalar (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The scalar to multiply by.
     * @return The result of multiplying the vector by the scalar.
     */
    friend constexpr Vec3<T> operator*(Vec3<T> lhs, T rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies a vector by a scalar (component-wise).
     *
     * @param[in] lhs The scalar to multiply by.
     * @param[in] rhs The vector to scale.
     * @return The result of multiplying the vector by the scalar.
     */
    friend constexpr Vec3<T> operator*(T lhs, Vec3<T> rhs) noexcept {
        rhs *= lhs;
        return rhs;
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

    /**
     * @brief Computes the euclidean norm (or magnitude) of this vector.
     *
     * @return The euclidean norm (or magnitude) of this vector.
     */
    constexpr T norm() const noexcept {
        return std::sqrt(this->dot(*this));
    }

    /**
     * @brief Computes the normalized (unit) vector of this vector.
     *
     * @return The normalized (unit) vector of this vector.
     */
    constexpr Vec3<T> normalize() const noexcept {
        T n = norm();
        return { x / n, y / n, z / n };
    }

    /**
     * @brief Computes the reflection of this vector around a surface normal.
     *
     * @param[in] normal The surface normal to reflect around.
     * @return The reflected vector.
     */
    constexpr Vec3<T> reflect(const Vec3<T>& normal) const noexcept {
        return static_cast<T>(2) * normal * normal.dot(*this) - *this;
    }

};

}

#endif