#ifndef CGFS_CAMERA_HPP
#define CGFS_CAMERA_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include "cgfs/quat.hpp"
#include "cgfs/space.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief An adjustable camera. */
class Camera final {
public:

    /** @brief The default position of the camera in world space. */
    static constexpr Vec3<f32> DEFAULT_POSITION = cgfs::Vec3<f32>::splat(0.0F);

    /** @brief The default viewing direction of the camera in world space. */
    static constexpr Vec3<f32> DEFAULT_DIRECTION = { 0.0F, 0.0F, -1.0F };

    /** @brief The default vertical field of view (in radians). */
    static constexpr f32 DEFAULT_FOV = cgfs::radians(60.0F);

    /** @brief The default minimum vertical field of view (in radians). */
    static constexpr f32 DEFAULT_MIN_FOV = cgfs::radians(1.0F);

    /** @brief The default maximum vertical field of view (in radians). */
    static constexpr f32 DEFAULT_MAX_FOV = cgfs::radians(179.0F);

    /** @brief The default minimum pitch angle (in radians). */
    static constexpr f32 DEFAULT_MIN_PITCH = cgfs::radians(-89.0F);

    /** @brief The default maximum pitch angle (in radians). */
    static constexpr f32 DEFAULT_MAX_PITCH = cgfs::radians(89.0F);

    /** @brief A builder for configuring and creating a camera. */
    class Builder final {
    private:

        /** @brief The position in world space. */
        Vec3<f32> _position = DEFAULT_POSITION;

        /** @brief The viewing direction in world space. */
        Vec3<f32> _direction = DEFAULT_DIRECTION;

        /** @brief The vertical field of view (in radians). */
        f32 _fov = DEFAULT_FOV;

        /** @brief The minimum vertical field of view (in radians). */
        f32 _minFov = DEFAULT_MIN_FOV;

        /** @brief The maximum vertical field of view (in radians). */
        f32 _maxFov = DEFAULT_MAX_FOV;

        /** @brief The minimum pitch angle (in radians). */
        f32 _minPitch = DEFAULT_MIN_PITCH;

        /** @brief The maximum pitch angle (in radians). */
        f32 _maxPitch = DEFAULT_MAX_PITCH;

    public:

        /**
         * @brief Sets the position in world space.
         *
         * @param[in] position The position in world space.
         * @return A reference to this builder.
         *
         * @note By default, the camera is positioned at
         * `Camera::DEFAULT_POSITION`.
         */
        constexpr Builder& position(const Vec3<f32>& position) noexcept {
            _position = position;
            return *this;
        }

        /**
         * @brief Sets the orientation in world space to look in a specified
         * direction.
         *
         * @param[in] direction The direction to look towards.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `direction` has a norm of zero.
         *
         * @note By default, the camera looks towards
         * `Camera::DEFAULT_DIRECTION`.
         */
        constexpr Builder& look(const Vec3<f32>& direction) noexcept {
            assert(direction.norm() > 0.0F);
            _direction = direction;
            return *this;
        }

        /**
         * @brief Sets the vertical field of view (in radians).
         *
         * @param[in] fov The vertical field of view (in radians).
         * @return A reference to this builder.
         *
         * @note By default, the camera has a vertical field of view of
         * `Camera::DEFAULT_FOV`.
         */
        constexpr Builder& fov(f32 fov) noexcept {
            _fov = fov;
            return *this;
        }

        /**
         * @brief Sets the minimum and maximum vertical field of view (in
         * radians).
         *
         * @param[in] minFov The minimum vertical field of view (in radians).
         * @param[in] maxFov The maximum vertical field of view (in radians).
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `minFov` is less than or equal
         * to `0.0F`, if `maxFov` is greater than or equal to
         * `cgfs::radians(180.0F)`, or if `minFov` is greater than `maxFov`.
         *
         * @note By default, the camera has a minimum vertical field of view of
         * `Camera::DEFAULT_MIN_FOV`, and a maximum vertical field of view of
         * `Camera::DEFAULT_MAX_FOV`.
         */
        constexpr Builder& fov_limits(f32 minFov, f32 maxFov) noexcept {
            assert((minFov > 0.0F) && (maxFov < cgfs::radians(180.0F)));
            assert(minFov <= maxFov);
            _minFov = minFov;
            _maxFov = maxFov;
            return *this;
        }

        /**
         * @brief Sets the minimum and maximum pitch angles (in radians).
         *
         * @param[in] minPitch The minimum pitch angle (in radians).
         * @param[in] maxPitch The maximum pitch angle (in radians).
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `minPitch` is less than or
         * equal to `cgfs::radians(-90.0F)`, if `maxPitch` is greater than or
         * equal to `cgfs::radians(90.0F)`, or if `minPitch` is greater than
         * `maxPitch`.
         *
         * @note By default, the camera has a minimum pitch angle of
         * `Camera::DEFAULT_MIN_PITCH`, and a maximum pitch angle of
         * `Camera::DEFAULT_MAX_PITCH`.
         */
        constexpr Builder& pitch_limits(f32 minPitch, f32 maxPitch) noexcept {
            assert(
                (minPitch > cgfs::radians(-90.0F))
                    && (maxPitch < cgfs::radians(90.0F))
            );
            assert(minPitch <= maxPitch);
            _minPitch = minPitch;
            _maxPitch = maxPitch;
            return *this;
        }

        /**
         * @brief Returns a camera with the specified properties.
         *
         * @return A camera with the specified properties.
         */
        constexpr Camera build() const noexcept {
            Camera camera(
                _position,
                Quat<f32>::identity(),
                std::clamp(_fov, _minFov, _maxFov),
                _minFov,
                _maxFov,
                _minPitch,
                _maxPitch
            );
            camera.look(_direction);
            return camera;
        }

    };

private:

    /** @brief The position in world space. */
    Vec3<f32> _position;

    /** @brief The orientation in world space. */
    Quat<f32> _orientation;

    /** @brief The vertical field of view (in radians). */
    f32 _fov;

    /** @brief The minimum vertical field of view (in radians). */
    f32 _minFov;

    /** @brief The maximum vertical field of view (in radians). */
    f32 _maxFov;

    /** @brief The minimum pitch angle (in radians). */
    f32 _minPitch;

    /** @brief The maximum pitch angle (in radians). */
    f32 _maxPitch;

    /**
     * @brief Constructs a camera with the specified properties.
     *
     * @param[in] position    The position in world space.
     * @param[in] orientation The orientation in world space.
     * @param[in] fov         The vertical field of view (in radians).
     * @param[in] minFov      The minimum vertical field of view (in radians).
     * @param[in] maxFov      The maximum vertical field of view (in radians).
     * @param[in] minPitch    The minimum pitch angle (in radians).
     * @param[in] maxPitch    The maximum pitch angle (in radians).
     */
    constexpr Camera(
        const Vec3<f32>& position,
        const Quat<f32>& orientation,
        f32 fov,
        f32 minFov,
        f32 maxFov,
        f32 minPitch,
        f32 maxPitch
    ) noexcept
        : _position(position),
          _orientation(orientation),
          _fov(fov),
          _minFov(minFov),
          _maxFov(maxFov),
          _minPitch(minPitch),
          _maxPitch(maxPitch) { }

    /**
     * @brief Sets the orientation in world space using yaw and pitch angles.
     *
     * @param[in] yaw   The yaw angle (in radians).
     * @param[in] pitch The pitch angle (in radians).
     *
     * @warning The behavior is undefined if `yaw` or `pitch` is not finite.
     */
    constexpr void set_orientation(f32 yaw, f32 pitch) noexcept {
        assert(std::isfinite(yaw) && std::isfinite(pitch));
        pitch = std::clamp(pitch, _minPitch, _maxPitch);
        Quat<f32> quatYaw = Quat<f32>::from_axis_angle(
            { 0.0F, 1.0F, 0.0F },
            yaw
        );
        Quat<f32> quatPitch = Quat<f32>::from_axis_angle(
            { 1.0F, 0.0F, 0.0F },
            pitch
        );
        _orientation = quatYaw * quatPitch;
    }

public:

    /**
     * @brief Returns a camera builder.
     *
     * @return A camera builder.
     */
    static constexpr Builder builder() noexcept {
        return Builder();
    }

    /**
     * @brief Returns the position in world space.
     *
     * @return The position in world space.
     */
    constexpr const Vec3<f32>& position() const noexcept {
        return _position;
    }

    /**
     * @brief Sets the position in world space.
     *
     * @param[in] position The position in world space.
     */
    constexpr void set_position(const Vec3<f32>& position) noexcept {
        _position = position;
    }

    /**
     * @brief Adjusts the position in world space by a delta.
     *
     * @param[in] delta The change in position.
     * @param[in] space Whether the change is specified in view or world space.
     */
    constexpr void adjust_position(
        const Vec3<f32>& delta,
        Space space = Space::VIEW
    ) noexcept {
        _position += (space == Space::VIEW)
            ? _orientation.rotate(delta)
            : delta;
    }

    /**
     * @brief Returns the orientation in world space.
     *
     * @return The orientation in world space.
     */
    constexpr const Quat<f32>& orientation() const noexcept {
        return _orientation;
    }

    /**
     * @brief Returns the forward axis in world space.
     *
     * @return The forward axis in world space.
     */
    constexpr Vec3<f32> forward() const noexcept {
        return _orientation.rotate({ 0.0F, 0.0F, -1.0F });
    }

    /**
     * @brief Returns the up axis in world space.
     *
     * @return The up axis in world space.
     */
    constexpr Vec3<f32> up() const noexcept {
        return _orientation.rotate({ 0.0F, 1.0F, 0.0F });
    }

    /**
     * @brief Returns the right axis in world space.
     *
     * @return The right axis in world space.
     */
    constexpr Vec3<f32> right() const noexcept {
        return _orientation.rotate({ 1.0F, 0.0F, 0.0F });
    }

    /**
     * @brief Returns the pitch angle (in radians).
     *
     * @return The pitch angle (in radians).
     */
    constexpr f32 pitch() const noexcept {
        return std::asin(std::clamp(forward().y, -1.0F, 1.0F));
    }

    /**
     * @brief Returns the minimum pitch angle (in radians).
     *
     * @return The minimum pitch angle (in radians).
     */
    constexpr f32 min_pitch() const noexcept {
        return _minPitch;
    }

    /**
     * @brief Returns the maximum pitch angle (in radians).
     *
     * @return The maximum pitch angle (in radians).
     */
    constexpr f32 max_pitch() const noexcept {
        return _maxPitch;
    }

    /**
     * @brief Sets the pitch angle (in radians).
     *
     * @param[in] pitch The pitch angle (in radians).
     */
    constexpr void set_pitch(f32 pitch) noexcept {
        set_orientation(yaw(), pitch);
    }

    /**
     * @brief Adjusts the pitch angle by a delta (in radians).
     *
     * @param[in] delta The change in pitch angle (in radians).
     */
    constexpr void adjust_pitch(f32 delta) noexcept {
        set_orientation(yaw(), pitch() + delta);
    }

    /**
     * @brief Returns the yaw angle (in radians).
     *
     * @return The yaw angle (in radians).
     */
    constexpr f32 yaw() const noexcept {
        Vec3<f32> f = forward();
        return std::atan2(-f.x, -f.z);
    }

    /**
     * @brief Sets the yaw angle (in radians).
     *
     * @param[in] yaw The yaw angle (in radians).
     */
    constexpr void set_yaw(f32 yaw) noexcept {
        set_orientation(yaw, pitch());
    }

    /**
     * @brief Adjusts the yaw angle by a delta (in radians).
     *
     * @param[in] delta The change in yaw angle (in radians).
     */
    constexpr void adjust_yaw(f32 delta) noexcept {
        set_orientation(yaw() + delta, pitch());
    }

    /**
     * @brief Sets the orientation in world space to look in a specified
     * direction.
     *
     * @param[in] direction The direction to look towards.
     *
     * @warning The behavior is undefined if `direction` has a norm of zero.
     */
    constexpr void look(const Vec3<f32>& direction) noexcept {
        assert(direction.norm() > 0.0F);
        Vec3<f32> f = direction.normalize();
        f32 yaw = std::atan2(-f.x, -f.z);
        f32 pitch = std::asin(std::clamp(f.y, -1.0F, 1.0F));
        set_orientation(yaw, pitch);
    }

    /**
     * @brief Returns the vertical field of view (in radians).
     *
     * @return The vertical field of view (in radians).
     */
    constexpr f32 fov() const noexcept {
        return _fov;
    }

    /**
     * @brief Returns the minimum vertical field of view (in radians).
     *
     * @return The minimum vertical field of view (in radians).
     */
    constexpr f32 min_fov() const noexcept {
        return _minFov;
    }

    /**
     * @brief Returns the maximum vertical field of view (in radians).
     *
     * @return The maximum vertical field of view (in radians).
     */
    constexpr f32 max_fov() const noexcept {
        return _maxFov;
    }

    /**
     * @brief Sets the vertical field of view (in radians).
     *
     * @param[in] fov The vertical field of view (in radians).
     *
     * @warning The behavior is undefined if `fov` is not finite.
     */
    constexpr void set_fov(f32 fov) noexcept {
        assert(std::isfinite(fov));
        _fov = std::clamp(fov, _minFov, _maxFov);
    }

    /**
     * @brief Adjusts the vertical field of view by a delta (in radians).
     *
     * @param[in] delta The change in vertical field of view (in radians).
     */
    constexpr void adjust_fov(f32 delta) noexcept {
        set_fov(_fov + delta);
    }

};

}

#endif