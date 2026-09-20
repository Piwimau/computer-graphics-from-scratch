#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include "cgfs/mat3.hpp"
#include "cgfs/quat.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a camera. */
class Camera final {
private:

    /** @brief The position of this camera. */
    Vec3<f32> _position;

    /** @brief The orientation of this camera. */
    Quat<f32> _orientation;

    /**
     * @brief Initializes a camera with a specified position and orientation.
     *
     * @param[in] position    The position of the camera.
     * @param[in] orientation The orientation of the camera.
     */
    constexpr Camera(
        const Vec3<f32>& position,
        const Quat<f32>& orientation
    ) noexcept
        : _position(position),
          _orientation(orientation) { }
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
        return Camera(position, Quat<f32>::from_mat3(rotation));
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
     * @brief Returns the orientation of this camera.
     *
     * @return The orientation of this camera.
     */
    constexpr const Quat<f32>& orientation() const noexcept {
        return _orientation;
    }

    /**
     * @brief Returns the right axis (or positive x-axis) of this camera in
     * world space.
     *
     * @return The right axis (or positive x-axis) of this camera in world
     * space.
     */
    constexpr Vec3<f32> right() const noexcept {
        return _orientation.rotate({ 1.0F, 0.0F, 0.0F });
    }

    /**
     * @brief Returns the up axis (or positive y-axis) of this camera in world
     * space.
     *
     * @return The up axis (or positive y-axis) of this camera in world space.
     */
    constexpr Vec3<f32> up() const noexcept {
        return _orientation.rotate({ 0.0F, 1.0F, 0.0F });
    }

    /**
     * @brief Returns the forward axis (or negative z-axis) of this camera in
     * world space.
     *
     * @return The forward axis (or negative z-axis) of this camera in world
     * space.
     */
    constexpr Vec3<f32> forward() const noexcept {
        return _orientation.rotate({ 0.0F, 0.0F, -1.0F });
    }

    /**
     * @brief Returns the rotation of this camera in world space.
     *
     * @return The rotation of this camera in world space.
     */
    constexpr Mat3<f32> rotation() const noexcept {
        return _orientation.to_mat3();
    }

    /**
     * @brief Translates this camera by an offset in local space.
     *
     * @param[in] offset The offset by which to translate.
     */
    constexpr void translate_local(const Vec3<f32>& offset) noexcept {
        _position += _orientation.rotate(offset);
    }

    /**
     * @brief Translates this camera by an offset in world space.
     *
     * @param[in] offset The offset by which to translate.
     */
    constexpr void translate_world(const Vec3<f32>& offset) noexcept {
        _position += offset;
    }

    /**
     * @brief Rotates this camera around an axis in local space.
     *
     * @param[in] axis  The axis to rotate around.
     * @param[in] angle The rotation angle (in radians).
     */
    constexpr void rotate_local(const Vec3<f32>& axis, f32 angle) noexcept {
        Quat<f32> rotation = Quat<f32>::from_axis_angle(axis, angle);
        _orientation = (_orientation * rotation).normalize();
    }

    /**
     * @brief Rotates this camera around an axis in world space.
     *
     * @param[in] axis  The axis to rotate around.
     * @param[in] angle The rotation angle (in radians).
     */
    constexpr void rotate_world(const Vec3<f32>& axis, f32 angle) noexcept {
        Quat<f32> rotation = Quat<f32>::from_axis_angle(axis, angle);
        _orientation = (rotation * _orientation).normalize();
    }

    /**
     * @brief Rotates this camera around the local right axis.
     *
     * @param[in] angle The rotation angle (in radians).
     */
    constexpr void pitch(f32 angle) noexcept {
        rotate_local({ 1.0F, 0.0F, 0.0F }, angle);
    }

    /**
     * @brief Rotates this camera around the world up axis.
     *
     * @param[in] angle The rotation angle (in radians).
     */
    constexpr void yaw(f32 angle) noexcept {
        rotate_world({ 0.0F, 1.0F, 0.0F }, angle);
    }

    /**
     * @brief Rotates this camera around the local forward axis.
     *
     * @param[in] angle The rotation angle (in radians).
     */
    constexpr void roll(f32 angle) noexcept {
        rotate_local({ 0.0F, 0.0F, -1.0F }, angle);
    }

};

}

#endif