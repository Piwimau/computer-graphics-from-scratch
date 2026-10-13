#ifndef CGFS_DIELECTRIC_BSDF_HPP
#define CGFS_DIELECTRIC_BSDF_HPP

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>
#include "bounce.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"
#include "prng.hpp"

namespace cgfs {

/** @brief A BSDF for a dielectric material. */
class DielectricBsdf final {
private:

    /** @brief The surface normal (normalized). */
    Vec3<f32> _normal;

    /** @brief The direction towards the camera (normalized). */
    Vec3<f32> _view;

    /**
     * @brief The ratio of the indices of refraction (incident / transmitted).
     */
    f32 _eta;

    /** @brief The cosine of the incident angle. */
    f32 _cosI;

    /** @brief The cosine of the transmitted angle. */
    f32 _cosT;

    /** @brief The Fresnel reflectance. */
    f32 _fresnel;

    /**
     * @brief Returns the Fresnel reflectance and the cosine of the transmitted
     * angle.
     *
     * @param[in] eta  The ratio of the indices of refraction (incident /
     *                 transmitted).
     * @param[in] cosI The cosine of the incident angle.
     * @return The Fresnel reflectance and the cosine of the transmitted angle.
     */
    static constexpr std::pair<f32, f32> fresnel(f32 eta, f32 cosI) noexcept {
        f32 sinSquaredT = eta * eta * (1.0F - cosI * cosI);
        if (sinSquaredT >= 1.0F) {
            return { 1.0F, 0.0F };
        }
        f32 cosT = std::sqrt(1.0F - sinSquaredT);
        f32 rs = (eta * cosI - cosT) / std::max(eta * cosI + cosT, 1.0E-12F);
        f32 rp = (cosI - eta * cosT) / std::max(cosI + eta * cosT, 1.0E-12F);
        return { 0.5F * (rs * rs + rp * rp), cosT };
    }

public:

    /**
     * @brief Constructs a BSDF for a dielectric material.
     *
     * @param[in] normal The surface normal (normalized).
     * @param[in] view   The direction towards the camera (normalized).
     * @param[in] eta    The ratio of the indices of refraction (incident /
     *                   transmitted).
     */
    constexpr DielectricBsdf(
        const Vec3<f32>& normal,
        const Vec3<f32>& view,
        f32 eta
    ) noexcept
        : _normal(normal),
          _view(view),
          _eta(eta),
          _cosI(std::clamp(normal.dot(view), 0.0F, 1.0F)) {
        std::tie(_fresnel, _cosT) = fresnel(_eta, _cosI);
    }

    /**
     * @brief Returns the fraction of light that crosses the interface without
     * being deflected, as seen by shadow rays.
     *
     * @return The fraction of light that crosses the interface.
     */
    constexpr f32 straight_transmittance() const noexcept {
        auto [f, _] = fresnel(std::min(_eta, 1.0F / _eta), _cosI);
        return 1.0F - f;
    }

    /**
     * @brief Samples a reflection or a refraction.
     *
     * @param[in, out] prng A pseudorandom number generator.
     * @return The sampled bounce.
     */
    constexpr Bounce sample(Xoshiro128StarStar& prng) const noexcept {
        if (prng.next_f32() < _fresnel) {
            return {
                .direction = _view.reflect(_normal),
                .weight = Color::splat(1.0F),
                .pdf = std::nullopt,
                .transmitted = false
            };
        }
        return {
            .direction = -_view * _eta + _normal * (_eta * _cosI - _cosT),
            .weight = Color::splat(1.0F),
            .pdf = std::nullopt,
            .transmitted = true
        };
    }

};

}

#endif