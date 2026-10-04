#ifndef CGFS_LIGHT_HPP
#define CGFS_LIGHT_HPP

#include <cassert>
#include <variant>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A light in a scene. */
class Light final {
public:

    /** @brief Attenuation factors for a point or spot light. */
    struct Attenuation {

        /** @brief The constant attenuation factor. */
        f32 constant;

        /** @brief The linear attenuation factor. */
        f32 linear;

        /** @brief The quadratic attenuation factor. */
        f32 quadratic;

    };

    /** @brief A point light radiating from a fixed position. */
    struct Point {

        /** @brief The position in world space. */
        Vec3<f32> position;

        /** @brief The radius. */
        f32 radius;

        /** @brief The attenuation factors. */
        Attenuation attenuation;

    };

    /** @brief A spot light radiating in the shape of a cone. */
    struct Spot {

        /** @brief The position in world space. */
        Vec3<f32> position;

        /** @brief The direction in world space. */
        Vec3<f32> direction;

        /** @brief The inner cutoff angle (in radians). */
        f32 innerCutoff;

        /** @brief The outer cutoff angle (in radians). */
        f32 outerCutoff;

        /** @brief The radius. */
        f32 radius;

        /** @brief The attenuation factors. */
        Attenuation attenuation;

    };

    /** @brief A directional light radiating in a fixed direction. */
    struct Directional {

        /** @brief The direction in world space. */
        Vec3<f32> direction;

        /** @brief The angular radius (in radians). */
        f32 angularRadius;

    };

    /** @brief The kind of light. */
    using Kind = std::variant<Point, Spot, Directional>;

    /** @brief The default color of a light. */
    static constexpr Color DEFAULT_COLOR = Color::splat(1.0F);

    /** @brief The default intensity of a light. */
    static constexpr f32 DEFAULT_INTENSITY = 1.0F;

    /** @brief The default radius of a point or spot light. */
    static constexpr f32 DEFAULT_RADIUS = 1.0F;

    /**
     * @brief The default angular radius of a directional light (in radians).
     */
    static constexpr f32 DEFAULT_ANGULAR_RADIUS = cgfs::radians(0.5F);

    /** @brief The default position of a point or spot light. */
    static constexpr Vec3<f32> DEFAULT_POSITION = Vec3<f32>::splat(0.0F);

    /** @brief The default direction of a spot or directional light. */
    static constexpr Vec3<f32> DEFAULT_DIRECTION = { 0.0F, -1.0F, 0.0F };

    /** @brief The default inner cutoff angle of a spot light (in radians). */
    static constexpr f32 DEFAULT_INNER_CUTOFF = cgfs::radians(35.0F);

    /** @brief The default outer cutoff angle of a spot light (in radians). */
    static constexpr f32 DEFAULT_OUTER_CUTOFF = cgfs::radians(45.0F);

    /** @brief The default attenuation factors of a point or spot light. */
    static constexpr Attenuation DEFAULT_ATTENUATION = {
        .constant = 1.0F,
        .linear = 0.09F,
        .quadratic = 0.032F
    };

    /**
     * @brief A builder for lights.
     *
     * @tparam Derived The type of the derived builder.
     */
    template<typename Derived>
    class Builder {
    protected:

        /** @brief The color of the light. */
        Color _color = DEFAULT_COLOR;

        /** @brief The intensity of the light. */
        f32 _intensity = DEFAULT_INTENSITY;

    public:

        /**
         * @brief Sets the color of the light.
         *
         * @param[in] color The color of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if any component of `color` is
         * outside the range `[0.0F, 1.0F]`.
         */
        constexpr Derived& color(const Color& color) noexcept {
            assert((color.min() >= 0.0F) && (color.max() <= 1.0F));
            _color = color;
            return static_cast<Derived&>(*this);
        }

        /**
         * @brief Sets the intensity of the light.
         *
         * @param[in] intensity The intensity of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `intensity` is negative.
         */
        constexpr Derived& intensity(f32 intensity) noexcept {
            assert(intensity >= 0.0F);
            _intensity = intensity;
            return static_cast<Derived&>(*this);
        }

    };

    /** @brief A builder for point lights. */
    class PointBuilder final : public Builder<PointBuilder> {
    private:

        /** @brief The position of the light in world space. */
        Vec3<f32> _position = DEFAULT_POSITION;

        /** @brief The radius of the light. */
        f32 _radius = DEFAULT_RADIUS;

        /** @brief The attenuation factors of the light. */
        Attenuation _attenuation = DEFAULT_ATTENUATION;

    public:

        /**
         * @brief Sets the position of the light in world space.
         *
         * @param[in] position The position of the light in world space.
         * @return A reference to this builder.
         */
        constexpr PointBuilder& position(const Vec3<f32>& position) noexcept {
            _position = position;
            return *this;
        }

        /**
         * @brief Sets the radius of the light.
         *
         * @param[in] radius The radius of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `radius` is negative.
         */
        constexpr PointBuilder& radius(f32 radius) noexcept {
            assert(radius >= 0.0F);
            _radius = radius;
            return *this;
        }

        /**
         * @brief Sets the attenuation factors of the light.
         *
         * @param[in] constant  The constant attenuation factor of the light.
         * @param[in] linear    The linear attenuation factor of the light.
         * @param[in] quadratic The quadratic attenuation factor of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `constant`, `linear`, or
         * `quadratic` is negative.
         */
        constexpr PointBuilder& attenuation(
            f32 constant,
            f32 linear,
            f32 quadratic
        ) noexcept {
            assert(constant >= 0.0F);
            assert(linear >= 0.0F);
            assert(quadratic >= 0.0F);
            _attenuation.constant = constant;
            _attenuation.linear = linear;
            _attenuation.quadratic = quadratic;
            return *this;
        }

        /**
         * @brief Builds the point light.
         *
         * @return The point light.
         */
        constexpr Light build() const noexcept {
            return Light(
                _color,
                _intensity,
                Point { _position, _radius, _attenuation }
            );
        }

    };

    /** @brief A builder for spot lights. */
    class SpotBuilder final : public Builder<SpotBuilder> {
    private:

        /** @brief The position of the light in world space. */
        Vec3<f32> _position = DEFAULT_POSITION;

        /** @brief The direction of the light in world space. */
        Vec3<f32> _direction = DEFAULT_DIRECTION;

        /** @brief The inner cutoff angle of the light (in radians). */
        f32 _innerCutoff = DEFAULT_INNER_CUTOFF;

        /** @brief The outer cutoff angle of the light (in radians). */
        f32 _outerCutoff = DEFAULT_OUTER_CUTOFF;

        /** @brief The radius of the light. */
        f32 _radius = DEFAULT_RADIUS;

        /** @brief The attenuation factors of the light. */
        Attenuation _attenuation = DEFAULT_ATTENUATION;

    public:

        /**
         * @brief Sets the position of the light in world space.
         *
         * @param[in] position The position of the light in world space.
         * @return A reference to this builder.
         */
        constexpr SpotBuilder& position(const Vec3<f32>& position) noexcept {
            _position = position;
            return *this;
        }

        /**
         * @brief Sets the direction of the light in world space.
         *
         * @param[in] direction The direction of the light in world space.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `direction` has a norm of zero.
         */
        constexpr SpotBuilder& direction(const Vec3<f32>& direction) noexcept {
            assert(direction.norm() > 0.0F);
            _direction = direction.normalize();
            return *this;
        }

        /**
         * @brief Sets the inner and outer cutoff angles of the light (in
         * radians).
         *
         * @param[in] innerCutoff The inner cutoff angle of the light (in
         *                        radians).
         * @param[in] outerCutoff The outer cutoff angle of the light (in
         *                        radians).
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `innerCutoff` or `outerCutoff`
         * is less than or equal to `0.0F` or greater than or equal to
         * `cgfs::radians(180.0F)`, or if `innerCutoff` is greater than
         * `outerCutoff`.
         */
        constexpr SpotBuilder& cutoffs(
            f32 innerCutoff,
            f32 outerCutoff
        ) noexcept {
            assert(
                (innerCutoff > 0.0F) && (innerCutoff < cgfs::radians(180.0F))
            );
            assert(
                (outerCutoff > 0.0F) && (outerCutoff < cgfs::radians(180.0F))
            );
            assert(innerCutoff <= outerCutoff);
            _innerCutoff = innerCutoff;
            _outerCutoff = outerCutoff;
            return *this;
        }

        /**
         * @brief Sets the radius of the light.
         *
         * @param[in] radius The radius of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `radius` is negative.
         */
        constexpr SpotBuilder& radius(f32 radius) noexcept {
            assert(radius >= 0.0F);
            _radius = radius;
            return *this;
        }

        /**
         * @brief Sets the attenuation factors of the light.
         *
         * @param[in] constant  The constant attenuation factor of the light.
         * @param[in] linear    The linear attenuation factor of the light.
         * @param[in] quadratic The quadratic attenuation factor of the light.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `constant`, `linear`, or
         * `quadratic` is negative.
         */
        constexpr SpotBuilder& attenuation(
            f32 constant,
            f32 linear,
            f32 quadratic
        ) noexcept {
            assert(constant >= 0.0F);
            assert(linear >= 0.0F);
            assert(quadratic >= 0.0F);
            _attenuation.constant = constant;
            _attenuation.linear = linear;
            _attenuation.quadratic = quadratic;
            return *this;
        }

        /**
         * @brief Builds the spot light.
         *
         * @return The spot light.
         */
        constexpr Light build() const noexcept {
            return Light(
                _color,
                _intensity,
                Spot {
                    _position,
                    _direction,
                    _innerCutoff,
                    _outerCutoff,
                    _radius,
                    _attenuation
                }
            );
        }

    };

    /** @brief A builder for directional lights. */
    class DirectionalBuilder final : public Builder<DirectionalBuilder> {
    private:

        /** @brief The direction of the light in world space. */
        Vec3<f32> _direction = DEFAULT_DIRECTION;

        /** @brief The angular radius of the light (in radians). */
        f32 _angularRadius = DEFAULT_ANGULAR_RADIUS;

    public:

        /**
         * @brief Sets the direction of the light in world space.
         *
         * @param[in] direction The direction of the light in world space.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `direction` has a norm of zero.
         */
        constexpr DirectionalBuilder& direction(
            const Vec3<f32>& direction
        ) noexcept {
            assert(direction.norm() > 0.0F);
            _direction = direction.normalize();
            return *this;
        }

        /**
         * @brief Sets the angular radius of the light (in radians).
         *
         * @param[in] angularRadius The angular radius of the light (in
         *                          radians).
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `angularRadius` is negative or
         * greater than or equal to `cgfs::radians(90.0F)`.
         */
        constexpr DirectionalBuilder& angular_radius(
            f32 angularRadius
        ) noexcept {
            assert(
                (angularRadius >= 0.0F)
                    && (angularRadius < cgfs::radians(90.0F))
            );
            _angularRadius = angularRadius;
            return *this;
        }

        /**
         * @brief Builds the directional light.
         *
         * @return The directional light.
         */
        constexpr Light build() const noexcept {
            return Light(
                _color,
                _intensity,
                Directional { _direction, _angularRadius }
            );
        }

    };

private:

    /** @brief The color. */
    Color _color;

    /** @brief The intensity. */
    f32 _intensity;

    /** @brief The kind. */
    Kind _kind;

    /**
     * @brief Constructs a light with the specified properties.
     *
     * @param[in] color     The color of the light.
     * @param[in] intensity The intensity of the light.
     * @param[in] kind      The kind of the light.
     */
    constexpr Light(const Color& color, f32 intensity, const Kind& kind)
        : _color(color),
          _intensity(intensity),
          _kind(kind) { }

public:

    /**
     * @brief Creates a builder for a point light.
     *
     * @return A builder for a point light.
     */
    static constexpr PointBuilder point() noexcept {
        return PointBuilder();
    }

    /**
     * @brief Creates a builder for a spot light.
     *
     * @return A builder for a spot light.
     */
    static constexpr SpotBuilder spot() noexcept {
        return SpotBuilder();
    }

    /**
     * @brief Creates a builder for a directional light.
     *
     * @return A builder for a directional light.
     */
    static constexpr DirectionalBuilder directional() noexcept {
        return DirectionalBuilder();
    }

    /**
     * @brief Returns the color of this light.
     *
     * @return The color of this light.
     */
    constexpr Color color() const noexcept {
        return _color;
    }

    /**
     * @brief Returns the intensity of this light.
     *
     * @return The intensity of this light.
     */
    constexpr f32 intensity() const noexcept {
        return _intensity;
    }

    /**
     * @brief Returns the kind of this light.
     *
     * @return The kind of this light.
     */
    constexpr Kind kind() const noexcept {
        return _kind;
    }

    /**
     * @brief Tries to get the point light properties.
     *
     * @return The point light properties if this light is a point light,
     * otherwise `nullptr`.
     */
    constexpr const Point* as_point() const noexcept {
        return std::get_if<Point>(&_kind);
    }

    /**
     * @brief Tries to get the spot light properties.
     *
     * @return The spot light properties if this light is a spot light,
     * otherwise `nullptr`.
     */
    constexpr const Spot* as_spot() const noexcept {
        return std::get_if<Spot>(&_kind);
    }

    /**
     * @brief Tries to get the directional light properties.
     *
     * @return The directional light properties if this light is a directional
     * light, otherwise `nullptr`.
     */
    constexpr const Directional* as_directional() const noexcept {
        return std::get_if<Directional>(&_kind);
    }

};

}

#endif