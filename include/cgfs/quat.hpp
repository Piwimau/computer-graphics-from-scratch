#ifndef CGFS_QUAT_HPP
#define CGFS_QUAT_HPP

#include <cmath>
#include <concepts>
#include "cgfs/mat3.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/**
 * @brief Represents a quaternion.
 *
 * @tparam T The type of the quaternion's components.
 */
template<std::floating_point T>
struct Quat {

    /** @brief The x-component of this quaternion. */
    T x;

    /** @brief The y-component of this quaternion. */
    T y;

    /** @brief The z-component of this quaternion. */
    T z;

    /** @brief The w-component of this quaternion. */
    T w;

    /**
     * @brief Returns the identity quaternion.
     *
     * @return The identity quaternion.
     */
    static constexpr Quat<T> identity() noexcept {
        return { 0, 0, 0, 1 };
    }

    /**
     * @brief Creates a quaternion from an axis and angle of rotation.
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
        return { v.x, v.y, v.z, std::cos(halfAngle) };
    }

    /**
     * @brief Creates a quaternion from a 3x3 rotation matrix.
     *
     * @param[in] m The rotation matrix.
     * @return A quaternion representing the rotation described by the matrix.
     */
    static constexpr Quat<T> from_mat3(const Mat3<T>& m) noexcept {
        T trace = m.trace();
        if (trace >= static_cast<T>(0)) {
            T s = std::sqrt(trace + static_cast<T>(1)) * static_cast<T>(2);
            return {
                (m[2][1] - m[1][2]) / s,
                (m[0][2] - m[2][0]) / s,
                (m[1][0] - m[0][1]) / s,
                static_cast<T>(0.25) * s
            };
        }
        if ((m[0][0] >= m[1][1]) && (m[0][0] >= m[2][2])) {
            T s = std::sqrt(static_cast<T>(1) + m[0][0] - m[1][1] - m[2][2])
                * static_cast<T>(2);
            return {
                static_cast<T>(0.25) * s,
                (m[0][1] + m[1][0]) / s,
                (m[0][2] + m[2][0]) / s,
                (m[2][1] - m[1][2]) / s
            };
        }
        if (m[1][1] >= m[2][2]) {
            T s = std::sqrt(static_cast<T>(1) + m[1][1] - m[0][0] - m[2][2])
                * static_cast<T>(2);
            return {
                (m[0][1] + m[1][0]) / s,
                static_cast<T>(0.25) * s,
                (m[1][2] + m[2][1]) / s,
                (m[0][2] - m[2][0]) / s
            };
        }
        T s = std::sqrt(static_cast<T>(1) + m[2][2] - m[0][0] - m[1][1])
            * static_cast<T>(2);
        return {
            (m[0][2] + m[2][0]) / s,
            (m[1][2] + m[2][1]) / s,
            static_cast<T>(0.25) * s,
            (m[1][0] - m[0][1]) / s
        };
    }

    /**
     * @brief Adds a quaternion to this quaternion (component-wise).
     *
     * @param[in] rhs The quaternion to add.
     * @return A reference to this quaternion after the addition.
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
     * @return A reference to this quaternion after the subtraction.
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
     * @note This is the standard multiplication of two quaternions (also known
     * as the Hamilton product), not a component-wise one. Similar to matrix
     * multiplication, the order is reversed (i.e., the rotation of `rhs` is
     * applied first, followed by the rotation of `*this`).
     *
     * @param[in] rhs The quaternion to multiply by.
     * @return A reference to this quaternion after the multiplication.
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
     * @note This is the standard multiplication of two quaternions (also known
     * as the Hamilton product), not a component-wise one. Similar to matrix
     * multiplication, the order is reversed (i.e., the rotation of `rhs` is
     * applied first, followed by the rotation of `lhs`).
     *
     * @param[in] lhs The quaternion to multiply.
     * @param[in] rhs The quaternion to multiply by.
     * @return The result of multiplying the two quaternions.
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
        return { x, y, z };
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
        return { x / n, y / n, z / n, w / n };
    }

    /**
     * @brief Returns the conjugate of this quaternion.
     *
     * @return The conjugate of this quaternion.
     */
    constexpr Quat<T> conjugate() const noexcept {
        return { -x, -y, -z, w };
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
            conj.x * invNorm2,
            conj.y * invNorm2,
            conj.z * invNorm2,
            conj.w * invNorm2
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
     * @param[in] a The quaternion at `t = 0.0`.
     * @param[in] b The quaternion at `t = 1.0`.
     * @param[in] t The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated quaternion.
     */
    static constexpr Quat<T> lerp(
        const Quat<T>& a,
        const Quat<T>& b,
        T t
    ) noexcept {
        Quat<T> na = a.normalize();
        Quat<T> nb = b.normalize();
        Quat<T> result = {
            na.x * (static_cast<T>(1) - t) + nb.x * t,
            na.y * (static_cast<T>(1) - t) + nb.y * t,
            na.z * (static_cast<T>(1) - t) + nb.z * t,
            na.w * (static_cast<T>(1) - t) + nb.w * t
        };
        return result.normalize();
    }

    /**
     * @brief Spherically interpolates between two quaternions.
     *
     * @param[in] a The quaternion at `t = 0.0`.
     * @param[in] b The quaternion at `t = 1.0`.
     * @param[in] t The interpolation factor in the range `[0.0, 1.0]`.
     * @return The interpolated quaternion.
     */
    static constexpr Quat<T> slerp(
        const Quat<T>& a,
        const Quat<T>& b,
        T t
    ) noexcept {
        Quat<T> na = a.normalize();
        Quat<T> nb = b.normalize();
        T cosTheta = na.dot(nb);
        if (std::abs(cosTheta) >= static_cast<T>(1)) {
            return na;
        }
        Quat<T> c = nb;
        if (cosTheta < static_cast<T>(0)) {
            cosTheta = -cosTheta;
            c = { -nb.x, -nb.y, -nb.z, -nb.w };
        }
        T sinTheta = std::sqrt(static_cast<T>(1) - cosTheta * cosTheta);
        if (sinTheta < static_cast<T>(0.001)) {
            return lerp(na, c, t);
        }
        T theta = std::acos(cosTheta);
        T wa = std::sin((static_cast<T>(1) - t) * theta) / sinTheta;
        T wb = std::sin(t * theta) / sinTheta;
        return {
            na.x * wa + c.x * wb,
            na.y * wa + c.y * wb,
            na.z * wa + c.z * wb,
            na.w * wa + c.w * wb
        };
    }

};

}

#endif