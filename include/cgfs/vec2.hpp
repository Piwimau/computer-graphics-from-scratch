#ifndef CGFS_VEC2_HPP
#define CGFS_VEC2_HPP

#include <algorithm>
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
     * @param[in] lhs The vector to add to.
     * @param[in] rhs The vector to add.
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
     * @param[in] lhs The vector to subtract from.
     * @param[in] rhs The vector to subtract.
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
     * @brief Multiplies this vector by another vector (component-wise).
     *
     * @param[in] rhs The vector to multiply by.
     * @return A reference to this vector after the multiplication.
     */
    constexpr Vec2<T>& operator*=(const Vec2<T>& rhs) noexcept {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    /**
     * @brief Multiplies two vectors (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The vector to multiply by.
     * @return The result of multiplying the two vectors.
     */
    friend constexpr Vec2<T> operator*(
        Vec2<T> lhs,
        const Vec2<T>& rhs
    ) noexcept {
        lhs *= rhs;
        return lhs;
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
     * @brief Divides this vector by another vector (component-wise).
     *
     * @param[in] rhs The vector to divide by.
     * @return A reference to this vector after the division.
     */
    constexpr Vec2<T>& operator/=(const Vec2<T>& rhs) noexcept {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    /**
     * @brief Divides two vectors (component-wise).
     *
     * @param[in] lhs The vector to divide.
     * @param[in] rhs The vector to divide by.
     * @return The result of dividing the two vectors.
     */
    friend constexpr Vec2<T> operator/(
        Vec2<T> lhs,
        const Vec2<T>& rhs
    ) noexcept {
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

    /**
     * @brief Clamps the components of this vector between a minimum and maximum
     * value.
     *
     * @param[in] min The minimum value.
     * @param[in] max The maximum value (inclusive).
     * @return The clamped vector.
     */
    constexpr Vec2<T> clamp(T min, T max) const noexcept {
        return { std::clamp(x, min, max), std::clamp(y, min, max) };
    }

    /**
     * @brief Computes the linear interpolation between two vectors.
     *
     * @param[in] a The vector at `t = 0.0`.
     * @param[in] b The vector at `t = 1.0`.
     * @param[in] t The interpolation parameter in the range `[0.0, 1.0]`.
     * @return The interpolated vector.
     */
    static constexpr Vec2<T> lerp(
        const Vec2<T>& a,
        const Vec2<T>& b,
        T t
    ) noexcept requires std::floating_point<T> {
        return a + (b - a) * t;
    }

};

}

#endif