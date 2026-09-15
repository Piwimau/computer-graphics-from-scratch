#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include "cgfs/mat3.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief Represents a camera. */
struct Camera {

    /** @brief The position of this camera. */
    Vec3<f64> pos;

    /** @brief The rotation of this camera. */
    Mat3<f64> rot;

    /**
     * @brief Creates a camera that looks in a specified direction from a
     * viewpoint.
     *
     * @note The coordinate system is right-handed, with the camera looking
     * along the negative z-axis.
     *
     * @param[in] pos The position of the camera.
     * @param[in] dir The direction the camera is looking in.
     * @param[in] up  The direction considered "up".
     * @return A camera looking in the specified direction from the viewpoint.
     */
    static constexpr Camera look(
        const Vec3<f64>& pos,
        const Vec3<f64>& dir,
        const Vec3<f64>& up = { 0.0, 1.0, 0.0 }
    ) noexcept {
        Vec3<f64> z = (-dir).normalize();
        Vec3<f64> x = up.cross(z).normalize();
        Vec3<f64> y = z.cross(x);
        return {
            .pos = pos,
            .rot = {
                x.x, y.x, z.x,
                x.y, y.y, z.y,
                x.z, y.z, z.z
            }
        };
    }

    /**
     * @brief Creates a camera that looks at a target from a viewpoint.
     *
     * @note The coordinate system is right-handed, with the camera looking
     * along the negative z-axis.
     *
     * @param[in] pos    The position of the camera.
     * @param[in] target The point the camera is looking at.
     * @param[in] up     The direction considered "up".
     * @return A camera looking at the target from the viewpoint.
     */
    static constexpr Camera look_at(
        const Vec3<f64>& pos,
        const Vec3<f64>& target,
        const Vec3<f64>& up = { 0.0, 1.0, 0.0 }
    ) noexcept {
        return look(pos, (target - pos).normalize(), up);
    }

};

}

#endif