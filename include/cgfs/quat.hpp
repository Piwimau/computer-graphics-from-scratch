#ifndef CGFS_QUAT_HPP
#define CGFS_QUAT_HPP

#include <cmath>
#include <concepts>
#include "cgfs/mat3.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/**
 * @brief A quaternion.
 *
 * @tparam T The type of the components.
 */
template<std::floating_point T>
struct Quat {

    /** @brief The x-component. */
    T x;

    /** @brief The y-component. */
    T y;

    /** @brief The z-component. */
    T z;

    /** @brief The w-component. */
    T w;

    /**
     * @brief Constructs an identity quaternion.
     *
     * @return An identity quaternion.
     */
    static constexpr Quat<T> identity() noexcept {
        return { .x = 0, .y = 0, .z = 0, .w = 1 };
    }

    /**
     * @brief Constructs a quaternion from an axis and angle of rotation.
     *
     * @param[in] axis  The axis of rotation.
     * @param[in] angle The angle of rotation (in radians).
     * @return A quaternion representing the rotation around the axis with the
     * specified angle.
     */
    static constexpr Quat<T> from_axis_angle(
        const Vec3<T>& axis,
        T angle
    ) noexcept {
        T halfAngle = angle * static_cast<T>(0.5);
        Vec3<T> v = axis.normalize() * std::sin(halfAngle);
        return { .x = v.x, .y = v.y, .z = v.z, .w = std::cos(halfAngle) };
    }

    /**
     * @brief Constructs a quaternion from a 3x3 rotation matrix.
     *
     * @param[in] m The rotation matrix.
     * @return A quaternion representing the rotation described by the matrix.
     */
    static constexpr Quat<T> from_mat3(const Mat3<T>& m) noexcept {
        T trace = m.trace();
        if (trace >= static_cast<T>(0)) {
            T s = std::sqrt(trace + static_cast<T>(1)) * static_cast<T>(2);
            return {
                .x = (m[2][1] - m[1][2]) / s,
                .y = (m[0][2] - m[2][0]) / s,
                .z = (m[1][0] - m[0][1]) / s,
                .w = static_cast<T>(0.25) * s
            };
        }
        if ((m[0][0] >= m[1][1]) && (m[0][0] >= m[2][2])) {
            T s = std::sqrt(static_cast<T>(1) + m[0][0] - m[1][1] - m[2][2])
                * static_cast<T>(2);
            return {
                .x = static_cast<T>(0.25) * s,
                .y = (m[0][1] + m[1][0]) / s,
                .z = (m[0][2] + m[2][0]) / s,
                .w = (m[2][1] - m[1][2]) / s
            };
        }
        if (m[1][1] >= m[2][2]) {
            T s = std::sqrt(static_cast<T>(1) + m[1][1] - m[0][0] - m[2][2])
                * static_cast<T>(2);
            return {
                .x = (m[0][1] + m[1][0]) / s,
                .y = static_cast<T>(0.25) * s,
                .z = (m[1][2] + m[2][1]) / s,
                .w = (m[0][2] - m[2][0]) / s
            };
        }
        T s = std::sqrt(static_cast<T>(1) + m[2][2] - m[0][0] - m[1][1])
            * static_cast<T>(2);
        return {
            .x = (m[0][2] + m[2][0]) / s,
            .y = (m[1][2] + m[2][1]) / s,
            .z = static_cast<T>(0.25) * s,
            .w = (m[1][0] - m[0][1]) / s
        };
    }

    /**
     * @brief Adds a quaternion to this quaternion (component-wise).
     *
     * @param[in] rhs The quaternion to add.
     * @return A reference to this quaternion.
     */
    constexpr Quat<T>& operator+=(const Quat<T>& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    /**
     * @brief Adds two quaternions (component-wise).
     *
     * @param[in] lhs The quaternion to add to.
     * @param[in] rhs The quaternion to add.
     * @return The result of adding the two quaternions together.
     */
    friend constexpr Quat<T> operator+(
        Quat<T> lhs,
        const Quat<T>& rhs
    ) noexcept {
        lhs += rhs;
        return lhs;
    }

    /**
     * @brief Subtracts a quaternion from this quaternion (component-wise).
     *
     * @param[in] rhs The quaternion to subtract.
     * @return A reference to this quaternion.
     */
    constexpr Quat<T>& operator-=(const Quat<T>& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    /**
     * @brief Subtracts two quaternions (component-wise).
     *
     * @param[in] lhs The quaternion to subtract from.
     * @param[in] rhs The quaternion to subtract.
     * @return The result of subtracting the second quaternion from the first.
     */
    friend constexpr Quat<T> operator-(
        Quat<T> lhs,
        const Quat<T>& rhs
    ) noexcept {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies this quaternion by another quaternion.
     *
     * @param[in] rhs The quaternion to multiply by.
     * @return A reference to this quaternion.
     *
     * @note This is the standard multiplication of two quaternions (also known
     * as the Hamilton product), not a component-wise one. Similar to matrix
     * multiplication, the order is reversed (i.e., the rotation of `rhs` is
     * applied first, followed by the rotation of `*this`).
     */
    constexpr Quat<T>& operator*=(const Quat<T>& rhs) noexcept {
        Vec3<T> v0 = imag();
        Vec3<T> v1 = rhs.imag();
        T w0 = w;
        T w1 = rhs.w;
        Vec3<T> v = w0 * v1 + w1 * v0 + v0.cross(v1);
        x = v.x;
        y = v.y;
        z = v.z;
        w = w0 * w1 - v0.dot(v1);
        return *this;
    }

    /**
     * @brief Multiplies two quaternions.
     *
     * @param[in] lhs The quaternion to multiply.
     * @param[in] rhs The quaternion to multiply by.
     * @return The result of multiplying the two quaternions.
     *
     * @note This is the standard multiplication of two quaternions (also known
     * as the Hamilton product), not a component-wise one. Similar to matrix
     * multiplication, the order is reversed (i.e., the rotation of `rhs` is
     * applied first, followed by the rotation of `lhs`).
     */
    friend constexpr Quat<T> operator*(
        Quat<T> lhs,
        const Quat<T>& rhs
    ) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Returns the real part of this quaternion.
     *
     * @return The real part of this quaternion.
     */
    constexpr T real() const noexcept {
        return w;
    }

    /**
     * @brief Returns the imaginary part of this quaternion.
     *
     * @return The imaginary part of this quaternion.
     */
    constexpr Vec3<T> imag() const noexcept {
        return { .x = x, .y = y, .z = z };
    }

    /**
     * @brief Returns the rotation angle of this quaternion.
     *
     * @return The rotation angle of this quaternion.
     */
    constexpr T angle() const noexcept {
        return static_cast<T>(2) * std::atan2(imag().norm(), w);
    }

    /**
     * @brief Returns the rotation axis of this quaternion.
     *
     * @return The rotation axis of this quaternion.
     */
    constexpr Vec3<T> axis() const noexcept {
        return imag().normalize();
    }

    /**
     * @brief Returns the dot product of this quaternion and another quaternion.
     *
     * @param[in] rhs The other quaternion.
     * @return The dot product of this quaternion and the other quaternion.
     */
    constexpr T dot(const Quat<T>& rhs) const noexcept {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    /**
     * @brief Returns the norm (or magnitude) of this quaternion.
     *
     * @return The norm (or magnitude) of this quaternion.
     */
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }

    /**
     * @brief Returns the normalized (unit) quaternion.
     *
     * @return The normalized quaternion.
     */
    constexpr Quat<T> normalize() const noexcept {
        T n = norm();
        return { .x = x / n, .y = y / n, .z = z / n, .w = w / n };
    }

    /**
     * @brief Returns the conjugate of this quaternion.
     *
     * @return The conjugate of this quaternion.
     */
    constexpr Quat<T> conjugate() const noexcept {
        return { .x = -x, .y = -y, .z = -z, .w = w };
    }

    /**
     * @brief Returns the inverse of this quaternion.
     *
     * @return The inverse of this quaternion.
     */
    constexpr Quat<T> inverse() const noexcept {
        Quat<T> conj = conjugate();
        T invNorm2 = static_cast<T>(1) / dot(*this);
        return {
            .x = conj.x * invNorm2,
            .y = conj.y * invNorm2,
            .z = conj.z * invNorm2,
            .w = conj.w * invNorm2
        };
    }

    /**
     * @brief Rotates a vector by this quaternion.
     *
     * @param[in] v The vector to rotate.
     * @return The rotated vector.
     */
    constexpr Vec3<T> rotate(const Vec3<T>& v) const noexcept {
        Vec3<T> u = imag();
        Vec3<T> rotated = v * (w * w - u.dot(u))
            + static_cast<T>(2) * u * u.dot(v)
            + static_cast<T>(2) * w * u.cross(v);
        return rotated / dot(*this);
    }

    /**
     * @brief Converts this quaternion to a 3x3 rotation matrix.
     *
     * @return The rotation matrix corresponding to this quaternion.
     */
    constexpr Mat3<T> to_mat3() const noexcept {
        T xx = x * x;
        T yy = y * y;
        T zz = z * z;
        T xy = x * y;
        T xz = x * z;
        T yz = y * z;
        T wx = w * x;
        T wy = w * y;
        T wz = w * z;
        T s = static_cast<T>(2) / dot(*this);
        return {
            static_cast<T>(1) - s * (yy + zz), s * (xy - wz), s * (xz + wy),
            s * (xy + wz), static_cast<T>(1) - s * (xx + zz), s * (yz - wx),
            s * (xz - wy), s * (yz + wx), static_cast<T>(1) - s * (xx + yy)
        };
    }

    /**
     * @brief Linearly interpolates between two quaternions.
     *
     * @param[in] lhs The quaternion at `t = 0.0`.
     * @param[in] rhs The quaternion at `t = 1.0`.
     * @param[in] t   The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated quaternion.
     */
    static constexpr Quat<T> lerp(
        const Quat<T>& lhs,
        const Quat<T>& rhs,
        T t
    ) noexcept {
        Quat<T> nl = lhs.normalize();
        Quat<T> nr = rhs.normalize();
        Quat<T> result = {
            .x = nl.x * (static_cast<T>(1) - t) + nr.x * t,
            .y = nl.y * (static_cast<T>(1) - t) + nr.y * t,
            .z = nl.z * (static_cast<T>(1) - t) + nr.z * t,
            .w = nl.w * (static_cast<T>(1) - t) + nr.w * t
        };
        return result.normalize();
    }

    /**
     * @brief Spherically interpolates between two quaternions.
     *
     * @param[in] lhs The quaternion at `t = 0.0`.
     * @param[in] rhs The quaternion at `t = 1.0`.
     * @param[in] t   The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated quaternion.
     */
    static constexpr Quat<T> slerp(
        const Quat<T>& lhs,
        const Quat<T>& rhs,
        T t
    ) noexcept {
        Quat<T> nl = lhs.normalize();
        Quat<T> nr = rhs.normalize();
        T cosTheta = nl.dot(nr);
        if (std::abs(cosTheta) >= static_cast<T>(1)) {
            return nl;
        }
        Quat<T> temp = nr;
        if (cosTheta < static_cast<T>(0)) {
            cosTheta = -cosTheta;
            temp = { -nr.x, -nr.y, -nr.z, -nr.w };
        }
        T sinTheta = std::sqrt(static_cast<T>(1) - cosTheta * cosTheta);
        if (sinTheta < static_cast<T>(0.001)) {
            return lerp(nl, temp, t);
        }
        T theta = std::acos(cosTheta);
        T wl = std::sin((static_cast<T>(1) - t) * theta) / sinTheta;
        T wr = std::sin(t * theta) / sinTheta;
        return {
            .x = nl.x * wl + temp.x * wr,
            .y = nl.y * wl + temp.y * wr,
            .z = nl.z * wl + temp.z * wr,
            .w = nl.w * wl + temp.w * wr
        };
    }

};

}

#endif