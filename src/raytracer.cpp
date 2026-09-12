#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <limits>
#include <optional>
#include <utility>
#include "cgfs/color.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents a ray of light. */
struct Ray {

    /** @brief The point at which this ray originates. */
    Vec3<f64> origin;

    /** @brief The direction this ray travels in. */
    Vec3<f64> direction;

};

/**
 * @brief Finds the closest intersection of a ray with a collection of spheres.
 *
 * @param[in] ray     The ray to test for intersections.
 * @param[in] spheres A collection of spheres to test for intersections.
 * @param[in] tMin    The minimum distance along the ray to consider for
 *                    intersections.
 * @param[in] tMax    The maximum distance along the ray to consider for
 *                    intersections.
 * @return A pair containing the closest intersected sphere and the distance
 * along the ray, or `std::nullopt` if no intersection is found.
 */
static std::optional<std::pair<Sphere, f64>> closest_intersection(
    const Ray& ray,
    std::span<const Sphere> spheres,
    f64 tMin,
    f64 tMax
) noexcept {
    std::optional<std::pair<Sphere, f64>> closest;
    for (const Sphere& sphere : spheres) {
        Vec3<f64> co = ray.origin - sphere.center;
        f64 a = ray.direction.dot(ray.direction);
        f64 b = 2.0 * co.dot(ray.direction);
        f64 c = co.dot(co) - sphere.radius * sphere.radius;
        f64 discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0.0) {
            continue;
        }
        f64 t1 = (-b - std::sqrt(discriminant)) / (2.0 * a);
        if ((t1 >= tMin) && (t1 <= tMax)) {
            if (!closest || (t1 < closest->second)) {
                closest = std::make_pair(sphere, t1);
            }
        }
        f64 t2 = (-b + std::sqrt(discriminant)) / (2.0 * a);
        if ((t2 >= tMin) && (t2 <= tMax)) {
            if (!closest || (t2 < closest->second)) {
                closest = std::make_pair(sphere, t2);
            }
        }
    }
    return closest;
}

/**
 * @brief Computes the intensity of the light illuminating a point with a
 * specified surface normal.
 *
 * @param[in] point     The point to compute the lighting for.
 * @param[in] normal    The surface normal at that point.
 * @param[in] viewDir   The viewing direction from the point towards the camera.
 * @param[in] shininess The shininess of the surface at the point (if any).
 * @param[in] spheres   A collection of spheres representing the scene.
 * @param[in] lights    A collection of lights illuminating the scene.
 * @return The intensity of the light illuminating the specified point.
 */
static f64 compute_lighting(
    const Vec3<f64>& point,
    const Vec3<f64>& normal,
    const Vec3<f64>& viewDir,
    std::optional<f64> shininess,
    std::span<const Sphere> spheres,
    std::span<const Light> lights
) noexcept {
    f64 intensity = 0.0;
    for (const Light& light : lights) {
        std::visit(
            [&]<typename T>(const T& l) {
                if constexpr (std::same_as<T, AmbientLight>) {
                    intensity += l.intensity;
                }
                else {
                    Vec3<f64> lightDir;
                    f64 tMax;
                    if constexpr (std::same_as<T, PointLight>) {
                        lightDir = l.position - point;
                        tMax = 1.0;
                    }
                    else {
                        lightDir = l.direction;
                        tMax = std::numeric_limits<f64>::infinity();
                    }
                    Ray shadowRay = { .origin = point, .direction = lightDir };
                    auto shadowIntersection = closest_intersection(
                        shadowRay,
                        spheres,
                        0.001,
                        tMax
                    );
                    if (shadowIntersection) {
                        return;
                    }
                    intensity += l.intensity
                        * std::max(normal.dot(lightDir), 0.0)
                        / (normal.norm() * lightDir.norm());
                    if (shininess) {
                        Vec3<f64> reflectDir = lightDir.reflect(normal);
                        intensity += l.intensity
                            * std::pow(
                                std::max(reflectDir.dot(viewDir), 0.0)
                                    / (reflectDir.norm() * viewDir.norm()),
                                *shininess
                            );
                    }
                }
            },
            light
        );
    }
    return intensity;
}

/**
 * @brief Traces a ray through a scene and returns the color of the closest
 * sphere it intersects with (if any).
 *
 * @param[in] ray     The ray to trace.
 * @param[in] spheres A collection of spheres representing the scene.
 * @param[in] lights  A collection of lights illuminating the scene.
 * @param[in] tMin    The minimum distance along the ray to consider for
 *                    intersections.
 * @param[in] tMax    The maximum distance along the ray to consider for
 *                    intersections.
 * @param[in] depth   The maximum recursion depth for reflective rays, `3` by
 *                    default.
 * @return The color of the closest sphere the ray intersects with, or black if
 * it does not intersect with any spheres.
 */
static Color trace_ray(
    const Ray& ray,
    std::span<const Sphere> spheres,
    std::span<const Light> lights,
    f64 tMin,
    f64 tMax,
    isize depth = 3
) noexcept {
    assert(depth >= 0);
    std::optional<std::pair<Sphere, f64>> closest = closest_intersection(
        ray,
        spheres,
        tMin,
        tMax
    );
    if (!closest) {
        return BLACK;
    }
    const auto& [closestSphere, tClosest] = *closest;
    Vec3<f64> point = ray.origin + tClosest * ray.direction;
    Vec3<f64> normal = (point - closestSphere.center).normalize();
    Color localColor = closestSphere.color
        * compute_lighting(
            point,
            normal,
            -ray.direction,
            closestSphere.shininess,
            spheres,
            lights
        );
    if ((depth == 0) || !closestSphere.reflectiveness) {
        return localColor;
    }
    Ray reflectRay = {
        .origin = point,
        .direction = (-ray.direction).reflect(normal)
    };
    Color reflectColor = trace_ray(
        reflectRay,
        spheres,
        lights,
        0.001,
        std::numeric_limits<f64>::infinity(),
        depth - 1
    );
    return Color::lerp(localColor, reflectColor, *closestSphere.reflectiveness);
}

void raytrace(
    std::span<const Sphere> spheres,
    std::span<const Light> lights,
    const Camera& camera,
    const Viewport& viewport,
    Canvas& canvas
) noexcept {
    for (isize y = canvas.height() / 2 - 1; y >= -canvas.height() / 2; y--) {
        for (isize x = -canvas.width() / 2; x <= canvas.width() / 2 - 1; x++) {
            Vec3<f64> pos = {
                .x = static_cast<f64>(x) * viewport.width
                    / static_cast<f64>(canvas.width()),
                .y = static_cast<f64>(y) * viewport.height
                    / static_cast<f64>(canvas.height()),
                .z = viewport.distance
            };
            Ray ray = {
                .origin = camera.position,
                .direction = pos - camera.position
            };
            canvas.draw_pixel(
                { x, y },
                trace_ray(
                    ray,
                    spheres,
                    lights,
                    1.0,
                    std::numeric_limits<f64>::infinity()
                )
            );
        }
    }
}

}