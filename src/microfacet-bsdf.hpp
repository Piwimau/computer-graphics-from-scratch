#ifndef CGFS_MICROFACET_BSDF_HPP
#define CGFS_MICROFACET_BSDF_HPP

#include <algorithm>
#include <cmath>
#include <optional>
#include "basis.hpp"
#include "bounce.hpp"
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"
#include "cgfs/vec3.hpp"
#include "prng.hpp"
#include "sampling.hpp"

namespace cgfs {

/** @brief A BSDF for a microfacet material. */
class MicrofacetBsdf final {
private:

    /** @brief The minimum GGX alpha to avoid degenerate lobes. */
    static constexpr f32 MIN_ALPHA = 0.001F;

    /** @brief The surface normal (normalized). */
    Vec3<f32> _normal;

    /** @brief The direction towards the camera (normalized). */
    Vec3<f32> _view;

    /** @brief The dot product of the normal and view directions. */
    f32 _nDotV;

    /** @brief The reflectance at normal incidence. */
    Color _f0;

    /** @brief The Fresnel reflectance. */
    Color _reflectance;

    /** @brief The diffuse color. */
    Color _diffuse;

    /** @brief The GGX alpha. */
    f32 _alpha;

    /** @brief The GGX alpha squared. */
    f32 _alphaSquared;

    /** @brief Whether the surface is a perfect mirror. */
    bool _mirror;

    /** @brief The probability of sampling the diffuse lobe. */
    f32 _diffuseProbability;

    /** @brief The probability of sampling the specular lobe. */
    f32 _specularProbability;

    /**
     * @brief Returns the Fresnel reflectance using Schlick's approximation.
     *
     * @param[in] f0       The reflectance at normal incidence.
     * @param[in] cosTheta The cosine of the angle between the two directions.
     * @return The Fresnel reflectance using Schlick's approximation.
     */
    static constexpr Color fresnel_schlick(
        const Color& f0,
        f32 cosTheta
    ) noexcept {
        f32 x = std::clamp(1.0F - cosTheta, 0.0F, 1.0F);
        return f0 + (Color::splat(1.0F) - f0) * (x * x * x * x * x);
    }

    /**
     * @brief Returns the Fresnel reflectance.
     *
     * @param[in] half The half vector between the view and light directions
     *                 (normalized).
     * @return The Fresnel reflectance.
     */
    constexpr Color fresnel(const Vec3<f32>& half) const noexcept {
        return _mirror ? _reflectance : fresnel_schlick(_f0, _view.dot(half));
    }

    /**
     * @brief Returns the GGX normal distribution.
     *
     * @param[in] half The half vector between view and light directions
     *                 (normalized).
     * @return The GGX normal distribution.
     */
    constexpr f32 ggx_distribution(const Vec3<f32>& half) const noexcept {
        Vec3<f32> nCrossH = _normal.cross(half);
        f32 nDotH = _normal.dot(half);
        f32 k = _alpha / (nCrossH.dot(nCrossH) + nDotH * nDotH * _alphaSquared);
        return k * k / PI<f32>;
    }

    /**
     * @brief Samples the diffuse lobe.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return The sampled bounce.
     */
    constexpr Bounce sample_diffuse(Xoshiro128StarStar& prng) const noexcept {
        Vec3<f32> direction = sample_cosine_hemisphere(_normal, prng);
        Vec3<f32> half = (_view + direction).normalize();
        return {
            .direction = direction,
            .weight = _diffuse * (Color::splat(1.0F) - fresnel(half))
                * (1.0F / _diffuseProbability),
            .pdf = pdf(direction),
            .transmitted = false
        };
    }

    /**
     * @brief Samples a microfacet normal from the distribution of normals
     * visible from a direction.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return A microfacet normal (normalized).
     */
    constexpr Vec3<f32> sample_ggx_normal(
        Xoshiro128StarStar& prng
    ) const noexcept {
        Basis basis = Basis::around(_normal);
        Vec3<f32> v = basis.to_local(_view);
        Vec3<f32> stretched = {
            .x = _alpha * v.x,
            .y = _alpha * v.y,
            .z = v.z
        };
        stretched = stretched.normalize();
        f32 lengthSquared = stretched.x * stretched.x
            + stretched.y * stretched.y;
        Vec3<f32> t1 = (lengthSquared > 0.0F)
            ? Vec3<f32> { -stretched.y, stretched.x, 0.0F }
                * (1.0F / std::sqrt(lengthSquared))
            : Vec3<f32> { 1.0F, 0.0F, 0.0F };
        Vec3<f32> t2 = stretched.cross(t1);
        auto [x, y] = sample_unit_disk(prng);
        f32 p1 = x;
        f32 s = 0.5F * (1.0F + stretched.z);
        f32 p2 = (1.0F - s) * std::sqrt(1.0F - x * x) + s * y;
        Vec3<f32> h = t1 * p1 + t2 * p2
            + stretched * std::sqrt(std::max(1.0F - p1 * p1 - p2 * p2, 0.0F));
        Vec3<f32> local = {
            .x = _alpha * h.x,
            .y = _alpha * h.y,
            .z = std::max(h.z, 0.0F)
        };
        local = local.normalize();
        return basis.to_world(local);
    }

    /**
     * @brief Returns the Smith-GGX denominator for a single direction.
     *
     * @param[in] nDotX The cosine of the angle between the normal and
     *                  direction.
     * @return The Smith-GGX denominator for the direction.
     */
    constexpr f32 smith_denominator(f32 nDotX) const noexcept {
        return nDotX
            + std::sqrt(_alphaSquared + (1.0F - _alphaSquared) * nDotX * nDotX);
    }

    /**
     * @brief Samples the specular lobe.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return The sampled bounce, or `std::nullopt` if it is not usable.
     */
    constexpr std::optional<Bounce> sample_specular(
        Xoshiro128StarStar& prng
    ) const noexcept {
        if (_mirror) {
            return Bounce {
                .direction = _view.reflect(_normal),
                .weight = _reflectance * (1.0F / _specularProbability),
                .pdf = std::nullopt,
                .transmitted = false
            };
        }
        Vec3<f32> h = sample_ggx_normal(prng);
        Vec3<f32> l = _view.reflect(h);
        f32 nDotL = _normal.dot(l);
        if (nDotL <= 0.0F) {
            return std::nullopt;
        }
        return Bounce {
            .direction = l,
            .weight = fresnel(h) * (2.0F * nDotL / smith_denominator(nDotL))
                * (1.0F / _specularProbability),
            .pdf = pdf(l),
            .transmitted = false
        };
    }

public:

    /**
     * @brief Constructs a BSDF for a microfacet material.
     *
     * @param[in] f0        The reflectance at normal incidence.
     * @param[in] diffuse   The diffuse color of the material.
     * @param[in] roughness The roughness of the material.
     * @param[in] normal    The surface normal (normalized).
     * @param[in] view      The direction towards the camera (normalized).
     */
    constexpr MicrofacetBsdf(
        const Color& f0,
        const Color& diffuse,
        f32 roughness,
        const Vec3<f32>& normal,
        const Vec3<f32>& view
    ) noexcept
        : _normal(normal),
          _view(view),
          _nDotV(std::max(_normal.dot(view), 0.0F)),
          _f0(f0),
          _reflectance(fresnel_schlick(_f0, _nDotV)),
          _diffuse(diffuse),
          _alpha(std::max(roughness * roughness, MIN_ALPHA)),
          _alphaSquared(_alpha * _alpha),
          _mirror(roughness == 0.0F) {
        f32 specularEnergy = _reflectance.max();
        f32 diffuseEnergy = _diffuse.max() * (1.0F - specularEnergy);
        f32 totalEnergy = diffuseEnergy + specularEnergy;
        if (totalEnergy > 0.0F) {
            _diffuseProbability = diffuseEnergy / totalEnergy;
            _specularProbability = specularEnergy / totalEnergy;
        }
        else {
            _diffuseProbability = 0.0F;
            _specularProbability = 0.0F;
        }
    }

    /**
     * @brief Returns the value of this BSDF for a light direction.
     *
     * @param[in] light The direction towards the light (normalized).
     * @return The value of this BSDF for the light direction.
     */
    constexpr Color eval(const Vec3<f32>& light) const noexcept {
        Vec3<f32> half = (_view + light).normalize();
        Color f = fresnel(half);
        Color diffuse = _diffuse * (Color::splat(1.0F) - f) * (1.0F / PI<f32>);
        if (_mirror) {
            return diffuse;
        }
        f32 nDotL = std::max(_normal.dot(light), 0.0F);
        f32 specular = ggx_distribution(half)
            / (smith_denominator(_nDotV) * smith_denominator(nDotL));
        return diffuse + f * specular;
    }

    /**
     * @brief Returns the value of the PDF for a light direction.
     *
     * @param[in] light The direction towards the light (normalized).
     * @return The value of the PDF for the light direction.
     */
    constexpr f32 pdf(const Vec3<f32>& light) const noexcept {
        f32 nDotL = _normal.dot(light);
        if (nDotL <= 0.0F) {
            return 0.0F;
        }
        f32 value = _diffuseProbability * nDotL / PI<f32>;
        if (!_mirror) {
            Vec3<f32> half = (_view + light).normalize();
            value += _specularProbability * ggx_distribution(half)
                / (2.0F * smith_denominator(_nDotV));
        }
        return value;
    }

    /**
     * @brief Samples the diffuse or specular lobe.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return The sampled bounce, or `std::nullopt` if it is not usable.
     */
    constexpr std::optional<Bounce> sample(
        Xoshiro128StarStar& prng
    ) const noexcept {
        if (_diffuseProbability <= 0.0F && _specularProbability <= 0.0F) {
            return std::nullopt;
        }
        return (prng.next_f32() < _specularProbability)
            ? sample_specular(prng)
            : sample_diffuse(prng);
    }

};

}

#endif