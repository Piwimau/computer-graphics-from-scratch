#ifndef CGFS_VEC2_HPP
#define CGFS_VEC2_HPP

#include <cmath>
#include <concepts>
#include <optional>
#include "cgfs/numeric.hpp"

namespace cgfs {

/**
 * @brief Represents a two-dimensional vector.
 *
 * @tparam T The type of the vector's components.
 */
template<Numeric T>
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
     * @brief Divides this vector by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to divide by.
     * @return A reference to this vector after the division.
     */
    constexpr Vec2<T>& operator/=(T rhs) noexcept {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    /**
     * @brief Divides a vector by a scalar (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The scalar to divide by.
     * @return The result of dividing the vector by the scalar.
     */
    friend constexpr Vec2<T> operator/(Vec2<T> lhs, T rhs) noexcept {
        lhs /= rhs;
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

    /**
     * @brief Computes the euclidean norm (or magnitude) of this vector.
     *
     * @return The euclidean norm (or magnitude) of this vector.
     */
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }

    /**
     * @brief Computes the angle between this vector and another vector (in
     * radians).
     *
     * @param[in] rhs The other vector.
     * @return The angle between this vector and the other vector (in radians).
     */
    constexpr T angle(const Vec2<T>& rhs) const noexcept
        requires std::floating_point<T> {
        return std::acos(dot(rhs) / (norm() * rhs.norm()));
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

    /**
     * @brief Computes the refraction of this vector through a surface with a
     * specified normal and refractive index ratio using Snell's law.
     *
     * @param[in] normal The surface normal to refract through.
     * @param[in] eta    The ratio of the incident and transmitted refractive
     *                   indices.
     * @return The refracted vector, or `std::nullopt` if total internal
     * reflection occurs.
     */
    constexpr std::optional<Vec2<T>> refract(
        const Vec2<T>& normal,
        T eta
    ) const noexcept requires std::floating_point<T> {
        T cosThetaI = -normal.dot(*this);
        T sin2ThetaI = eta * eta * (static_cast<T>(1) - cosThetaI * cosThetaI);
        if (sin2ThetaI > static_cast<T>(1)) {
            return std::nullopt;
        }
        T cosThetaT = std::sqrt(static_cast<T>(1) - sin2ThetaI);
        return eta * *this + (eta * cosThetaI - cosThetaT) * normal;
    }

};

}

#endif