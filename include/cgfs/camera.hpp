#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include "cgfs/mat3.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a camera. */
class Camera final {
private:

    /** @brief The position of this camera. */
    Vec3<f32> _position;

    /** @brief The rotation of this camera. */
    Mat3<f32> _rotation;

    /**
     * @brief Initializes a camera with a specified position and rotation.
     *
     * @param[in] position The position of the camera.
     * @param[in] rotation The rotation of the camera.
     */
    constexpr Camera(
        const Vec3<f32>& position,
        const Mat3<f32>& rotation
    ) noexcept
        : _position(position),
          _rotation(rotation) { }

public:

    /**
     * @brief Creates a camera at a position looking in a specified direction.
     *
     * @note The camera uses a right-handed coordinate system with the negative
     * z-axis pointing in the direction the camera is looking.
     *
     * @warning The behavior is undefined if `direction` has a length of zero,
     * or if `direction` is parallel to `up`.
     *
     * @param[in] position  The position of the camera.
     * @param[in] direction The direction the camera is looking at.
     * @param[in] up        The direction considered "up".
     * @return A camera at the position looking in the specified direction.
     */
    static constexpr Camera look(
        const Vec3<f32>& position,
        const Vec3<f32>& direction,
        const Vec3<f32>& up = { 0.0F, 1.0F, 0.0F }
    ) noexcept {
        Vec3<f32> z = (-direction).normalize();
        Vec3<f32> x = up.cross(z).normalize();
        Vec3<f32> y = z.cross(x);
        Mat3<f32> rotation = {
            x.x, y.x, z.x,
            x.y, y.y, z.y,
            x.z, y.z, z.z
        };
        return Camera(position, rotation);
    }

    /**
     * @brief Creates a camera at a position looking at a specified target.
     *
     * @note The camera uses a right-handed coordinate system with the negative
     * z-axis pointing in the direction the camera is looking.
     *
     * @warning The behavior is undefined if `target` is equal to `position`,
     * or if `target - position` is parallel to `up`.
     *
     * @param[in] position The position of the camera.
     * @param[in] target   The point the camera is looking at.
     * @param[in] up       The direction considered as "up".
     * @return A camera at the position looking at the specified target.
     */
    static constexpr Camera look_at(
        const Vec3<f32>& position,
        const Vec3<f32>& target,
        const Vec3<f32>& up = { 0.0F, 1.0F, 0.0F }
    ) noexcept {
        return look(position, target - position, up);
    }

    /**
     * @brief Returns the position of this camera.
     *
     * @return The position of this camera.
     */
    constexpr const Vec3<f32>& position() const noexcept {
        return _position;
    }

    /**
     * @brief Returns the rotation of this camera.
     *
     * @return The rotation of this camera.
     */
    constexpr const Mat3<f32>& rotation() const noexcept {
        return _rotation;
    }

};

}

#endif