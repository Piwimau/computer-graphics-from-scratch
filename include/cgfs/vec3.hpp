#ifndef CGFS_VEC3_HPP
#define CGFS_VEC3_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <optional>

namespace cgfs {

/**
 * @brief A three-dimensional vector.
 *
 * @tparam T The type of the components.
 */
template<std::floating_point T>
struct Vec3 {

    /** @brief The x-component. */
    T x;

    /** @brief The y-component. */
    T y;

    /** @brief The z-component. */
    T z;

    /**
     * @brief Constructs a vector with all components set to a specified value.
     *
     * @param[in] value The value for all components.
     * @return A vector with all components set to the specified value.
     */
    static constexpr Vec3<T> splat(T value) noexcept {
        return { .x = value, .y = value, .z = value };
    }

    /**
     * @brief Linearly interpolates between two vectors.
     *
     * @param[in] lhs The vector at `t = 0.0`.
     * @param[in] rhs The vector at `t = 1.0`.
     * @param[in] t   The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated vector.
     */
    static constexpr Vec3<T> lerp(
        const Vec3<T>& lhs,
        const Vec3<T>& rhs,
        T t
    ) noexcept {
        return lhs * (static_cast<T>(1) - t) + rhs * t;
    }

    /**
     * @brief Linearly interpolates between two vectors.
     *
     * @param[in] lhs The vector at `t = 0.0`.
     * @param[in] rhs The vector at `t = 1.0`.
     * @param[in] t   The interpolation factors for the components in the range
     *                `[0.0, 1.0]`.
     * @return The interpolated vector.
     */
    static constexpr Vec3<T> lerp(
        const Vec3<T>& lhs,
        const Vec3<T>& rhs,
        const Vec3<T>& t
    ) noexcept {
        return lhs * (Vec3<T>::splat(1) - t) + rhs * t;
    }

    /**
     * @brief Adds a vector to this vector (component-wise).
     *
     * @param[in] rhs The vector to add.
     * @return A reference to this vector.
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
     * @param[in] lhs The vector to add to.
     * @param[in] rhs The vector to add.
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
     * @return A reference to this vector.
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
     * @param[in] lhs The vector to subtract from.
     * @param[in] rhs The vector to subtract.
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
        return { .x = -x, .y = -y, .z = -z };
    }

    /**
     * @brief Multiplies this vector by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this vector.
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
     * @brief Multiplies this vector by another vector (component-wise).
     *
     * @param[in] rhs The vector to multiply by.
     * @return A reference to this vector.
     */
    constexpr Vec3<T>& operator*=(const Vec3<T>& rhs) noexcept {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    /**
     * @brief Multiplies two vectors (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The vector to multiply by.
     * @return The result of multiplying the two vectors.
     */
    friend constexpr Vec3<T> operator*(
        Vec3<T> lhs,
        const Vec3<T>& rhs
    ) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Divides this vector by a scalar (component-wise).
     *
     * @param[in] rhs The scalar to divide by.
     * @return A reference to this vector.
     */
    constexpr Vec3<T>& operator/=(T rhs) noexcept {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    /**
     * @brief Divides a vector by a scalar (component-wise).
     *
     * @param[in] lhs The vector to scale.
     * @param[in] rhs The scalar to divide by.
     * @return The result of dividing the vector by the scalar.
     */
    friend constexpr Vec3<T> operator/(Vec3<T> lhs, T rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    /**
     * @brief Divides this vector by another vector (component-wise).
     *
     * @param[in] rhs The vector to divide by.
     * @return A reference to this vector.
     */
    constexpr Vec3<T>& operator/=(const Vec3<T>& rhs) noexcept {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    /**
     * @brief Divides two vectors (component-wise).
     *
     * @param[in] lhs The vector to divide.
     * @param[in] rhs The vector to divide by.
     * @return The result of dividing the two vectors.
     */
    friend constexpr Vec3<T> operator/(
        Vec3<T> lhs,
        const Vec3<T>& rhs
    ) noexcept {
        lhs /= rhs;
        return lhs;
    }

    /**
     * @brief Returns the dot product of this vector and another vector.
     *
     * @param[in] rhs The other vector.
     * @return The dot product of this vector and the other vector.
     */
    constexpr T dot(const Vec3<T>& rhs) const noexcept {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    /**
     * @brief Returns the cross product of this vector and another vector.
     *
     * @param[in] rhs The other vector.
     * @return The cross product of this vector and the other vector.
     */
    constexpr Vec3<T> cross(const Vec3<T>& rhs) const noexcept {
        return {
            .x = y * rhs.z - z * rhs.y,
            .y = z * rhs.x - x * rhs.z,
            .z = x * rhs.y - y * rhs.x
        };
    }

    /**
     * @brief Returns the euclidean norm (or magnitude) of this vector.
     *
     * @return The euclidean norm (or magnitude) of this vector.
     */
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }

    /**
     * @brief Returns the angle between this vector and another vector (in
     * radians).
     *
     * @param[in] rhs The other vector.
     * @return The angle between this vector and the other vector (in radians).
     */
    constexpr T angle(const Vec3<T>& rhs) const noexcept {
        return std::acos(dot(rhs) / (norm() * rhs.norm()));
    }

    /**
     * @brief Returns the normalized (unit) vector of this vector.
     *
     * @return The normalized (unit) vector of this vector.
     */
    constexpr Vec3<T> normalize() const noexcept {
        T n = norm();
        return { .x = x / n, .y = y / n, .z = z / n };
    }

    /**
     * @brief Returns the reflection of this vector around a surface normal.
     *
     * @param[in] normal The surface normal to reflect around.
     * @return The reflected vector.
     */
    constexpr Vec3<T> reflect(const Vec3<T>& normal) const noexcept {
        return static_cast<T>(2) * normal * normal.dot(*this) - *this;
    }

    /**
     * @brief Returns the refraction of this vector through a surface with a
     * specified normal and refractive index ratio using Snell's law.
     *
     * @param[in] normal The surface normal to refract through.
     * @param[in] eta    The ratio of the incident and transmitted refractive
     *                   indices.
     * @return The refracted vector, or `std::nullopt` if total internal
     * reflection occurs.
     */
    constexpr std::optional<Vec3<T>> refract(
        const Vec3<T>& normal,
        T eta
    ) const noexcept {
        T cosThetaI = -normal.dot(*this);
        T sin2ThetaI = eta * eta * (static_cast<T>(1) - cosThetaI * cosThetaI);
        if (sin2ThetaI > static_cast<T>(1)) {
            return std::nullopt;
        }
        T cosThetaT = std::sqrt(static_cast<T>(1) - sin2ThetaI);
        return eta * *this + (eta * cosThetaI - cosThetaT) * normal;
    }

    /**
     * @brief Returns the minimum component of this vector.
     *
     * @return The minimum component of this vector.
     */
    constexpr T min() const noexcept {
        return std::min({ x, y, z });
    }

    /**
     * @brief Returns the minimum of this vector and another vector
     * (component-wise).
     *
     * @param[in] rhs The other vector.
     * @return The minimum of this vector and another vector (component-wise).
     */
    constexpr Vec3<T> min(const Vec3<T>& rhs) const noexcept {
        return {
            .x = std::min(x, rhs.x),
            .y = std::min(y, rhs.y),
            .z = std::min(z, rhs.z)
        };
    }

    /**
     * @brief Returns the maximum component of this vector.
     *
     * @return The maximum component of this vector.
     */
    constexpr T max() const noexcept {
        return std::max({ x, y, z });
    }

    /**
     * @brief Returns the maximum of this vector and another vector
     * (component-wise).
     *
     * @param[in] rhs The other vector.
     * @return The maximum of this vector and another vector (component-wise).
     */
    constexpr Vec3<T> max(const Vec3<T>& rhs) const noexcept {
        return {
            .x = std::max(x, rhs.x),
            .y = std::max(y, rhs.y),
            .z = std::max(z, rhs.z)
        };
    }

    /**
     * @brief Clamps the components of this vector to a specified range.
     *
     * @param[in] min The minimum value.
     * @param[in] max The maximum value (inclusive).
     * @return The clamped vector.
     */
    constexpr Vec3<T> clamp(T min, T max) const noexcept {
        return {
            .x = std::clamp(x, min, max),
            .y = std::clamp(y, min, max),
            .z = std::clamp(z, min, max)
        };
    }

    /**
     * @brief Clamps the components of this vector to a specified range.
     *
     * @param[in] min The minimum values for the components.
     * @param[in] max The maximum values for the components (inclusive).
     * @return The clamped vector.
     */
    constexpr Vec3<T> clamp(
        const Vec3<T>& min,
        const Vec3<T>& max
    ) const noexcept {
        return {
            .x = std::clamp(x, min.x, max.x),
            .y = std::clamp(y, min.y, max.y),
            .z = std::clamp(z, min.z, max.z)
        };
    }

};

}

#endif