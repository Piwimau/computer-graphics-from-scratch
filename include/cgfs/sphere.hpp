#ifndef CGFS_SPHERE_HPP
#define CGFS_SPHERE_HPP

#include <optional>
#include "cgfs/aabb.hpp"
#include "cgfs/hit.hpp"
#include "cgfs/prng.hpp"
#include "cgfs/ray.hpp"
#include "cgfs/sampling.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A sphere. */
struct Sphere {

    /** @brief The position in world space. */
    Vec3<f32> center;

    /** @brief The radius. */
    f32 radius;

    /** @brief The index of the surface material. */
    isize materialIdx;

    /**
     * @brief Returns the index of the surface material.
     *
     * @return The index of the surface material.
     */
    constexpr isize material_idx() const noexcept {
        return materialIdx;
    }

    /**
     * @brief Tries to intersect this sphere with a ray.
     *
     * @param[in] ray The ray to test for intersection.
     * @return The intersection between the ray and this sphere, or
     * `std::nullopt` if no intersection exists.
     *
     * @note The object index of the hit information is initialized to a dummy
     * value and must be filled in by the caller.
     */
    constexpr std::optional<Hit> intersect(const Ray& ray) const noexcept {
        Vec3<f32> f = ray.origin - center;
        f32 b = -f.dot(ray.direction);
        Vec3<f32> l = f + ray.direction * b;
        f32 d = radius * radius - l.norm_squared();
        if (d < 0.0F) {
            return std::nullopt;
        }
        f32 c = f.norm_squared() - radius * radius;
        f32 q = b + std::copysign(std::sqrt(d), b);
        f32 t0 = (q != 0.0F) ? c / q : 0.0F;
        f32 t1 = q;
        if (t0 > t1) {
            std::swap(t0, t1);
        }
        auto make_hit = [&](f32 distance) {
            Vec3<f32> position = ray.origin + ray.direction * distance;
            Vec3<f32> outwards = (position - center) * (1.0F / radius);
            position = center + outwards * radius;
            bool entering = ray.direction.dot(outwards) < 0.0F;
            return Hit {
                .distance = distance,
                .position = position,
                .normal = entering ? outwards : -outwards,
                .idx = -1,
                .entering = entering
            };
        };
        if (t0 > 0.0F && t0 < ray.maxDistance) {
            return make_hit(t0);
        }
        if (t1 > 0.0F && t1 < ray.maxDistance) {
            return make_hit(t1);
        }
        return std::nullopt;
    }

    /**
     * @brief Samples a point on this sphere.
     *
     * @param[in]      position The reference position for sampling.
     * @param[in, out] prng     A pseudorandom number generator.
     * @return The sampled point on this sphere on success, or `std::nullopt` on
     * failure.
     */
    constexpr std::optional<PointSample> sample_point(
        const Vec3<f32>& position,
        Xoshiro128StarStar& prng
    ) const noexcept {
        return sample_sphere(center, radius, position, prng);
    }

    /**
     * @brief Returns the PDF for sampling a point on this sphere.
     *
     * @param[in] position The reference position for sampling.
     * @return The PDF for sampling a point on this sphere on success, or
     * `std::nullopt` on failure.
     */
    constexpr std::optional<f32> sample_pdf(
        const Vec3<f32>& position
    ) const noexcept {
        f32 distanceSquared = (center - position).norm_squared();
        if (distanceSquared <= radius * radius) {
            return std::nullopt;
        }
        f32 versine = sphere_versine(distanceSquared, radius);
        return cone_pdf(versine);
    }

    /**
     * @brief Determines whether this sphere contains a position.
     *
     * @param[in] position The position to test.
     * @return `true` if this sphere contains the position, `false` otherwise.
     */
    constexpr bool contains(const Vec3<f32>& position) const noexcept {
        return (position - center).norm_squared() <= radius * radius;
    }

    /**
     * @brief Returns the axis-aligned bounding box of this sphere.
     *
     * @return The axis-aligned bounding box of this sphere.
     */
    constexpr Aabb bounds() const noexcept {
        Vec3<f32> v = Vec3<f32>::splat(radius);
        return Aabb { .min = center - v, .max = center + v };
    }

};

}

#endif