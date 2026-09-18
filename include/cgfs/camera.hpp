#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include "cgfs/mat3.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a camera. */
struct Camera {

    /** @brief The viewpoint of this camera. */
    Vec3<f32> viewpoint;

    /** @brief The rotation of this camera. */
    Mat3<f32> rotation;

    /**
     * @brief Creates a camera looking in a certain direction from a specified
     * viewpoint.
     *
     * @note The camera uses a right-handed coordinate system with the negative
     * z-axis pointing in the direction the camera is looking.
     *
     * @warning The behavior is undefined if `direction` has a length of zero,
     * or if `direction` is parallel to `up`.
     *
     * @param[in] viewpoint The viewpoint of the camera.
     * @param[in] direction The direction the camera is looking in.
     * @param[in] up        The direction considered as "up".
     * @return A camera looking in the direction from the specified viewpoint.
     */
    static constexpr Camera look(
        const Vec3<f32>& viewpoint,
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
        return { viewpoint, rotation };
    }

    /**
     * @brief Creates a camera looking at a target from a specified viewpoint.
     *
     * @note The camera uses a right-handed coordinate system with the negative
     * z-axis pointing in the direction the camera is looking.
     *
     * @warning The behavior is undefined if `target` is equal to `viewpoint`,
     * or if the direction from `viewpoint` to `target` is parallel to `up`.
     *
     * @param[in] viewpoint The viewpoint of the camera.
     * @param[in] target    The target the camera is looking at.
     * @param[in] up        The direction considered as "up".
     * @return A camera looking at the target from the specified viewpoint.
     */
    static constexpr Camera look_at(
        const Vec3<f32>& viewpoint,
        const Vec3<f32>& target,
        const Vec3<f32>& up = { 0.0F, 1.0F, 0.0F }
    ) noexcept {
        Vec3<f32> direction = (target - viewpoint).normalize();
        return look(viewpoint, direction, up);
    }

};

}

#endif