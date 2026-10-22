#ifndef CGFS_OBJECT_HPP
#define CGFS_OBJECT_HPP

#include <concepts>
#include <optional>
#include <variant>
#include "cgfs/aabb.hpp"
#include "cgfs/hit.hpp"
#include "cgfs/prng.hpp"
#include "cgfs/ray.hpp"
#include "cgfs/sampling.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/**
 * @brief An object that can be hit by a ray.
 *
 * @tparam T The type of the object.
 */
template<typename T>
concept Hittable = requires (
    const T& t,
    const Ray& ray,
    const Vec3<f32>& position,
    Xoshiro128StarStar& prng,
    const Hit& hit
) {
    { t.material_idx() } -> std::convertible_to<isize>;
    { t.intersect(ray) } -> std::same_as<std::optional<Hit>>;
    { t.sample_point(position, prng) } -> std::same_as<std::optional<PointSample>>;
    { t.sample_pdf(position) } -> std::same_as<std::optional<f32>>;
    { t.contains(position) } -> std::same_as<bool>;
    { t.bounds() } -> std::same_as<Aabb>;
};

/** @brief An object in a scene. */
class Object final {
public:

    /** @brief A kind of an object in a scene. */
    using Kind = std::variant<Sphere>;

private:

    /** @brief The kind of this object. */
    Kind _kind;

public:

    /**
     * @brief Constructs an object with a specified kind.
     *
     * @tparam T The type of the kind of the object.
     * @param[in] kind The kind of the object.
     */
    template<typename T>
    requires Hittable<T> && std::constructible_from<Kind, T&&>
    constexpr explicit(false) Object(
        T&& kind
    ) noexcept(std::is_nothrow_constructible_v<Kind, T&&>)
        : _kind(std::forward<T>(kind)) { }

    /**
     * @brief Returns the index of the material of this object.
     *
     * @return The index of the material of this object.
     */
    constexpr isize material_idx() const noexcept {
        return std::visit(
            [](const auto& kind) { return kind.material_idx(); },
            _kind
        );
    }

    /**
     * @brief Tries to intersect this object with a ray.
     *
     * @param[in] ray The ray to test for intersection.
     * @return The intersection between the ray and this object, or
     * `std::nullopt` if no intersection exists.
     */
    constexpr std::optional<Hit> intersect(const Ray& ray) const noexcept {
        return std::visit(
            [&](const auto& kind) { return kind.intersect(ray); },
            _kind
        );
    }

    /**
     * @brief Samples a point on this object.
     *
     * @param[in]      position The reference position for sampling.
     * @param[in, out] prng     A pseudorandom number generator.
     * @return The sampled point on this object on success, or `std::nullopt` on
     * failure.
     */
    constexpr std::optional<PointSample> sample_point(
        const Vec3<f32>& position,
        Xoshiro128StarStar& prng
    ) const noexcept {
        return std::visit(
            [&](const auto& kind) { return kind.sample_point(position, prng); },
            _kind
        );
    }

    /**
     * @brief Returns the PDF for sampling a point on this object.
     *
     * @param[in] position The reference position for sampling.
     * @return The PDF for sampling a point on this object on success, or
     * `std::nullopt` on failure.
     */
    constexpr std::optional<f32> sample_pdf(
        const Vec3<f32>& position
    ) const noexcept {
        return std::visit(
            [&](const auto& kind) { return kind.sample_pdf(position); },
            _kind
        );
    }

    /**
     * @brief Determines whether this object contains a position.
     *
     * @param[in] position The position to test.
     * @return `true` if this object contains the position, `false` otherwise.
     */
    constexpr bool contains(const Vec3<f32>& position) const noexcept {
        return std::visit(
            [&](const auto& kind) { return kind.contains(position); },
            _kind
        );
    }

    /**
     * @brief Returns the axis-aligned bounding box of this object.
     *
     * @return The axis-aligned bounding box of this object.
     */
    constexpr Aabb bounds() const noexcept {
        return std::visit(
            [](const auto& kind) { return kind.bounds(); },
            _kind
        );
    }

};

}

#endif