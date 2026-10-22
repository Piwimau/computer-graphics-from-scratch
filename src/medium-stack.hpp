#ifndef CGFS_MEDIUM_STACK_HPP
#define CGFS_MEDIUM_STACK_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <ranges>
#include <span>
#include "cgfs/color.hpp"
#include "cgfs/material.hpp"
#include "cgfs/object.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A medium a ray can travel through. */
struct Medium {

    /** @brief The index of the object bounding the medium. */
    isize ownerIdx;

    /** @brief The index of refraction. */
    f32 ior;

    /** @brief The absorption. */
    Color absorption;

};

/** @brief A stack for tracking nested media of transparent objects. */
class MediumStack final {
private:

    /** @brief The maximum nested transparent objects. */
    static constexpr isize MAX_NESTING = 8;

    /** @brief The medium outside of all objects. */
    static constexpr Medium VACUUM = {
        .ownerIdx = -1,
        .ior = 1.0F,
        .absorption = Color::splat(0.0F)
    };

    /** @brief The nested media (innermost last). */
    std::array<Medium, MAX_NESTING> _media = { };

    /** @brief The current size of this stack. */
    isize _size = 0;

    /**
     * @brief Returns the medium surrounding an object.
     *
     * @param[in] idx The index of the object whose surrounding medium to find.
     * @return The medium surrounding the object.
     */
    constexpr const Medium& surrounding(isize idx) const noexcept {
        auto end = _media.begin() + _size;
        auto it = std::ranges::find(
            _media.begin(),
            end,
            idx,
            &Medium::ownerIdx
        );
        if (it == end) {
            return current();
        }
        return (it == _media.begin()) ? VACUUM : *(it - 1);
    }

    /**
     * @brief Pushes the medium of a transparent object onto this stack.
     *
     * @param[in] idx         The index of the object whose medium to push.
     * @param[in] transparent The transparent material of the object.
     */
    constexpr void push(isize idx, const Material::Transparent& transparent) {
        assert(_size < MAX_NESTING);
        _media[_size++] = { idx, transparent.ior, transparent.absorption };
    }

public:

    /**
     * @brief Constructs an initial medium stack for a set of objects.
     *
     * @param[in] objects   The set of objects.
     * @param[in] materials The materials of the objects.
     * @param[in] position  The position of the camera.
     * @return The initial medium stack for the set of objects.
     */
    static constexpr MediumStack initial(
        std::span<const Object> objects,
        std::span<const Material> materials,
        const Vec3<f32>& position
    ) {
        std::array<isize, MAX_NESTING> enclosing;
        isize size = 0;
        for (auto [idx, object] : std::views::enumerate(objects)) {
            if (
                object.contains(position)
                    && materials[object.material_idx()].is_transparent()
            ) {
                assert(size < MAX_NESTING);
                enclosing[size++] = idx;
            }
        }
        std::ranges::sort(
            enclosing.begin(),
            enclosing.begin() + size,
            std::greater<f32>(),
            [&](isize idx) {
                Aabb box = objects[idx].bounds();
                return (box.max - box.min).norm_squared();
            }
        );
        MediumStack media;
        for (isize idx : std::span(enclosing.data(), size)) {
            const Object& object = objects[idx];
            const Material& material = materials[object.material_idx()];
            media.push(idx, *material.as_transparent());
        }
        return media;
    }

    /**
     * @brief Returns the medium the ray is currently traveling through.
     *
     * @return The medium the ray is currently traveling through.
     */
    constexpr const Medium& current() const noexcept {
        return (_size == 0) ? VACUUM : _media[_size - 1];
    }

    /**
     * @brief Returns the ratio of the indices of refraction at an interface.
     *
     * @param[in] idx         The index of the transparent object that was hit.
     * @param[in] transparent The transparent material of the object.
     * @param[in] entering    Whether the object is entered or left.
     * @return The ratio of the indices of refraction at the interface.
     */
    constexpr f32 eta(
        isize idx,
        const Material::Transparent& transparent,
        bool entering
    ) const noexcept {
        return entering
            ? current().ior / transparent.ior
            : transparent.ior / surrounding(idx).ior;
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
        if (_size == 0) {
            return Color::splat(1.0F);
        }
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
     * @brief Enters or leaves a transparent object.
     *
     * @param[in] idx         The index of the object that was hit.
     * @param[in] transparent The transparent material of the object.
     * @param[in] entering    Whether the object is entered or left.
     */
    constexpr void transition(
        isize idx,
        const Material::Transparent& transparent,
        bool entering
    ) {
        if (entering) {
            push(idx, transparent);
        }
        else {
            auto end = std::remove_if(
                _media.begin(),
                _media.begin() + _size,
                [&](const Medium& medium) { return medium.ownerIdx == idx; }
            );
            _size = end - _media.begin();
        }
    }

};

}

#endif