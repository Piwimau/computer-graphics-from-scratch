#ifndef CGFS_MATERIAL_HPP
#define CGFS_MATERIAL_HPP

#include <cassert>
#include <variant>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A material of a surface. */
class Material final {
public:

    /** @brief Properties unique to opaque materials. */
    struct Opaque {

        /** @brief The metalness. */
        f32 metalness;

    };

    /** @brief Properties unique to transparent materials. */
    struct Transparent {

        /** @brief The transparency. */
        f32 transparency;

        /** @brief The index of refraction. */
        f32 ior;

        /** @brief The absorption. */
        Color absorption;

    };

    /** @brief The kind of material. */
    using Kind = std::variant<Opaque, Transparent>;

    /** @brief The default albedo for a material. */
    static constexpr Color DEFAULT_ALBEDO = Color::splat(1.0F);

    /** @brief The default roughness for a material. */
    static constexpr f32 DEFAULT_ROUGHNESS = 1.0F;

    /** @brief The default metalness for an opaque material. */
    static constexpr f32 DEFAULT_METALNESS = 0.0F;

    /** @brief The default transparency for a transparent material. */
    static constexpr f32 DEFAULT_TRANSPARENCY = 1.0F;

    /** @brief The default index of refraction for a transparent material. */
    static constexpr f32 DEFAULT_IOR = 1.0F;

    /** @brief The default absorption for a transparent material. */
    static constexpr Color DEFAULT_ABSORPTION = Color::splat(0.0F);

    /**
     * @brief A builder for materials.
     *
     * @tparam Derived The type of the derived builder.
     */
    template<typename Derived>
    class Builder {
    protected:

        /** @brief The base color of the material. */
        Color _albedo = DEFAULT_ALBEDO;

        /** @brief The roughness of the material. */
        f32 _roughness = DEFAULT_ROUGHNESS;

    public:

        /**
         * @brief Sets the base color of the material.
         *
         * @param[in] albedo The base color of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if any component of `albedo` is
         * outside the range `[0.0F, 1.0F]`.
         */
        constexpr Derived& albedo(const Color& albedo) noexcept {
            assert((albedo.min() >= 0.0F) && (albedo.max() <= 1.0F));
            _albedo = albedo;
            return static_cast<Derived&>(*this);
        }

        /**
         * @brief Sets the roughness of the material.
         *
         * @param[in] roughness The roughness of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `roughness` is outside the
         * range `[0.0F, 1.0F]`.
         */
        constexpr Derived& roughness(f32 roughness) noexcept {
            assert((roughness >= 0.0F) && (roughness <= 1.0F));
            _roughness = roughness;
            return static_cast<Derived&>(*this);
        }

    };

    /** @brief A builder for opaque materials. */
    class OpaqueBuilder final : public Builder<OpaqueBuilder> {
    private:

        /** @brief The properties of the material. */
        Opaque _opaque = { .metalness = DEFAULT_METALNESS };

    public:

        /**
         * @brief Sets the metalness of the material.
         *
         * @param[in] metalness The metalness of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `metalness` is outside the
         * range `[0.0F, 1.0F]`.
         */
        constexpr OpaqueBuilder& metalness(f32 metalness) noexcept {
            assert((metalness >= 0.0F) && (metalness <= 1.0F));
            _opaque.metalness = metalness;
            return *this;
        }

        /**
         * @brief Builds the opaque material.
         *
         * @return The opaque material.
         */
        constexpr Material build() const noexcept {
            Color f0 = Color::lerp(
                { 0.04F, 0.04F, 0.04F },
                _albedo,
                _opaque.metalness
            );
            return Material(_albedo, f0, _roughness, _opaque);
        }

    };

    /** @brief A builder for transparent materials. */
    class TransparentBuilder final : public Builder<TransparentBuilder> {
    private:

        /** @brief The properties of the material. */
        Transparent _transparent = {
            .transparency = DEFAULT_TRANSPARENCY,
            .ior = DEFAULT_IOR,
            .absorption = DEFAULT_ABSORPTION
        };

    public:

        /**
         * @brief Sets the transparency of the material.
         *
         * @param[in] transparency The transparency of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `transparency` is outside the
         * range `[0.0F, 1.0F]`.
         */
        constexpr TransparentBuilder& transparency(f32 transparency) noexcept {
            assert((transparency >= 0.0F) && (transparency <= 1.0F));
            _transparent.transparency = transparency;
            return *this;
        }

        /**
         * @brief Sets the index of refraction of the material.
         *
         * @param[in] ior The index of refraction of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if `ior` is less than `1.0F`.
         */
        constexpr TransparentBuilder& ior(f32 ior) noexcept {
            assert(ior >= 1.0F);
            _transparent.ior = ior;
            return *this;
        }

        /**
         * @brief Sets the absorption of the material.
         *
         * @param[in] absorption The absorption of the material.
         * @return A reference to this builder.
         *
         * @warning The behavior is undefined if any component of `absorption`
         * is negative.
         */
        constexpr TransparentBuilder& absorption(
            const Color& absorption
        ) noexcept {
            assert(absorption.min() >= 0.0F);
            _transparent.absorption = absorption;
            return *this;
        }

        /**
         * @brief Builds the transparent material.
         *
         * @return The transparent material.
         */
        constexpr Material build() const noexcept {
            f32 x = (_transparent.ior - 1.0F) / (_transparent.ior + 1.0F);
            x = x * x;
            Color f0 = { x, x, x };
            return Material(_albedo, f0, _roughness, _transparent);
        }

    };

private:

    /** @brief The base color. */
    Color _albedo;

    /** @brief The reflectance at normal incidence. */
    Color _f0;

    /** @brief The roughness. */
    f32 _roughness;

    /** @brief The kind. */
    Kind _kind;

    /**
     * @brief Constructs a material with the specified properties.
     *
     * @param[in] albedo    The base color of the material.
     * @param[in] f0        The reflectance of the material at normal incidence.
     * @param[in] roughness The roughness of the material.
     * @param[in] kind      The kind of the material.
     */
    constexpr Material(
        const Color& albedo,
        const Color& f0,
        f32 roughness,
        const Kind& kind
    ) noexcept
        : _albedo(albedo),
          _f0(f0),
          _roughness(roughness),
          _kind(kind) { }

public:

    /**
     * @brief Constructs a builder for an opaque material.
     *
     * @return A builder for an opaque material.
     */
    static constexpr OpaqueBuilder opaque() noexcept {
        return OpaqueBuilder();
    }

    /**
     * @brief Constructs a builder for a transparent material.
     *
     * @return A builder for a transparent material.
     */
    static constexpr TransparentBuilder transparent() noexcept {
        return TransparentBuilder();
    }

    /**
     * @brief Returns the base color of this material.
     *
     * @return The base color of this material.
     */
    constexpr const Color& albedo() const noexcept {
        return _albedo;
    }

    /**
     * @brief Returns the reflectance of this material at normal incidence.
     *
     * @return The reflectance of this material at normal incidence.
     */
    constexpr const Color& f0() const noexcept {
        return _f0;
    }

    /**
     * @brief Returns the roughness of this material.
     *
     * @return The roughness of this material.
     */
    constexpr f32 roughness() const noexcept {
        return _roughness;
    }

    /**
     * @brief Returns the kind of this material.
     *
     * @return The kind of this material.
     */
    constexpr const Kind& kind() const noexcept {
        return _kind;
    }

    /**
     * @brief Tries to get the opaque properties of this material.
     *
     * @return The opaque properties if this material is opaque, otherwise
     * `nullptr`.
     */
    constexpr const Opaque* as_opaque() const noexcept {
        return std::get_if<Opaque>(&_kind);
    }

    /**
     * @brief Tries to get the transparent properties of this material.
     *
     * @return The transparent properties if this material is transparent,
     * otherwise `nullptr`.
     */
    constexpr const Transparent* as_transparent() const noexcept {
        return std::get_if<Transparent>(&_kind);
    }

};

}

#endif