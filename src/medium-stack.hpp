#ifndef CGFS_MEDIUM_STACK_HPP
#define CGFS_MEDIUM_STACK_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <span>
#include <utility>
#include <vector>
#include "cgfs/color.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A medium a ray can travel through. */
struct Medium {

    /** @brief The sphere bounding the medium. */
    const Sphere* owner;

    /** @brief The index of refraction. */
    f32 ior;

    /** @brief The absorption. */
    Color absorption;

};

/** @brief A stack for tracking nested media of transparent spheres. */
class MediumStack final {
private:

    /** @brief The medium outside of all spheres. */
    static constexpr Medium VACUUM = {
        .owner = nullptr,
        .ior = 1.0F,
        .absorption = Color::splat(0.0F),
    };

    /** @brief The nested media (innermost last). */
    std::vector<Medium> _media = { };

    /**
     * @brief Returns the medium surrounding a sphere.
     *
     * @param[in] owner The sphere whose surrounding medium to find.
     * @return The medium surrounding the sphere.
     */
    constexpr const Medium& surrounding(const Sphere& owner) const noexcept {
        auto it = std::ranges::find(_media, &owner, &Medium::owner);
        if (it == _media.end()) {
            return current();
        }
        return (it == _media.begin()) ? VACUUM : *(it - 1);
    }

    /**
     * @brief Pushes the medium of a transparent sphere onto this stack.
     *
     * @param[in] sphere The transparent sphere whose medium to push.
     *
     * @warning The behavior is undefined if `sphere` is not transparent.
     */
    constexpr void push(const Sphere& sphere) {
        assert(sphere.material.is_transparent());
        auto transparent = sphere.material.as_transparent();
        _media.emplace_back(&sphere, transparent->ior, transparent->absorption);
    }

public:

    /**
     * @brief Constructs an initial medium stack for a set of spheres.
     *
     * @param[in] spheres  The set of spheres.
     * @param[in] position The position of the camera.
     * @return The initial medium stack for the set of spheres.
     */
    static constexpr MediumStack initial(
        std::span<const Sphere> spheres,
        const Vec3<f32>& position
    ) {
        MediumStack media;
        for (const Sphere& sphere : spheres) {
            bool inside = (position - sphere.center).norm_squared()
                < sphere.radius * sphere.radius;
            if (sphere.material.is_transparent() && inside) {
                media.push(sphere);
            }
        }
        std::ranges::sort(
            media._media,
            std::greater<f32>(),
            [](const Medium& medium) { return medium.owner->radius; }
        );
        return media;
    }

    /**
     * @brief Returns the medium the ray is currently traveling through.
     *
     * @return The medium the ray is currently traveling through.
     */
    constexpr const Medium& current() const noexcept {
        return _media.empty() ? VACUUM : _media.back();
    }

    /**
     * @brief Returns the incident and transmitted indices of refraction at an
     * interface.
     *
     * @param[in] sphere   The transparent sphere that was hit.
     * @param[in] entering Whether the sphere is entered or left.
     * @return The incident and transmitted indices of refraction at the
     * interface.
     *
     * @warning The behavior is undefined if `sphere` is not transparent.
     */
    constexpr std::pair<f32, f32> iors(
        const Sphere& sphere,
        bool entering
    ) const noexcept {
        assert(sphere.material.is_transparent());
        auto transparent = sphere.material.as_transparent();
        f32 etaI = entering ? current().ior : transparent->ior;
        f32 etaT = entering ? transparent->ior : surrounding(sphere).ior;
        return { etaI, etaT };
    }

    /**
     * @brief Returns the fraction of light that survives traveling through the
     * current medium.
     *
     * @param[in] distance The distance traveled.
     * @return The fraction of light that survives traveling through the current
     * medium.
     */
    constexpr Color transmittance(f32 distance) const noexcept {
        distance = std::max(distance, 0.0F);
        Color absorption = current().absorption;
        auto f = [distance](f32 a) {
            return (a > 0.0F) ? std::exp(-a * distance) : 1.0F;
        };
        return {
            .r = f(absorption.r),
            .g = f(absorption.g),
            .b = f(absorption.b)
        };
    }

    /**
     * @brief Enters or leaves a transparent sphere.
     *
     * @param[in] sphere   The transparent sphere that was hit.
     * @param[in] entering Whether the sphere is entered or left.
     *
     * @warning The behavior is undefined if `sphere` is not transparent.
     */
    constexpr void transition(const Sphere& sphere, bool entering) {
        if (entering) {
            push(sphere);
        }
        else {
            std::erase_if(
                _media,
                [&](const Medium& medium) { return medium.owner == &sphere; }
            );
        }
    }

};

}

#endif