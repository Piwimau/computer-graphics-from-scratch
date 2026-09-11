#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>
#include <optional>
#include "cgfs/color.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"

namespace cgfs {

/** @brief Represents a ray of light. */
struct Ray {

    /** @brief The point at which this ray originates. */
    Vec3<f32> origin;

    /** @brief The direction this ray travels in. */
    Vec3<f32> direction;

};

/**
 * @brief Computes the intensity of the light illuminating a point with a
 * specified surface normal.
 *
 * @param[in] point     The point to compute the lighting for.
 * @param[in] normal    The surface normal at that point.
 * @param[in] viewDir   The viewing direction from the point towards the camera.
 * @param[in] shininess The shininess of the surface at the point (if any).
 * @param[in] lights    A collection of lights illuminating the scene.
 * @return The intensity of the light illuminating the specified point.
 */
static f32 compute_lighting(
    const Vec3<f32>& point,
    const Vec3<f32>& normal,
    const Vec3<f32>& viewDir,
    std::optional<f32> shininess,
    std::span<const Light> lights
) noexcept {
    f32 intensity = 0.0F;
    for (const Light& light : lights) {
        std::visit(
            [&]<typename T>(const T& l) {
                if constexpr (std::same_as<T, AmbientLight>) {
                    intensity += l.intensity;
                }
                else {
                    Vec3<f32> lightDir;
                    if constexpr (std::same_as<T, PointLight>) {
                        lightDir = l.position - point;
                    }
                    else {
                        lightDir = l.direction;
                    }
                    intensity += l.intensity
                        * std::max(normal.dot(lightDir), 0.0F)
                        / (normal.norm() * lightDir.norm());
                    if (shininess.has_value()) {
                        Vec3<f32> reflectDir = lightDir.reflect(normal);
                        intensity += l.intensity
                            * std::pow(
                                std::max(reflectDir.dot(viewDir), 0.0F)
                                    / (reflectDir.norm() * viewDir.norm()),
                                shininess.value()
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
 * object it intersects with.
 *
 * @param[in] ray     The ray to trace.
 * @param[in] spheres A collection of spheres representing the scene.
 * @param[in] lights  A collection of lights illuminating the scene.
 * @param[in] tMin    The minimum distance along the ray to consider for
 *                    intersections.
 * @param[in] tMax    The maximum distance along the ray to consider for
 *                    intersections.
 * @return The color of the closest object the ray intersects with, or black if
 * it does not intersect with any objects.
 */
static Color trace_ray(
    Ray ray,
    std::span<const Sphere> spheres,
    std::span<const Light> lights,
    f32 tMin,
    f32 tMax
) noexcept {
    f32 tClosest = std::numeric_limits<f32>::infinity();
    std::optional<Sphere> closestSphere;
    for (const Sphere& sphere : spheres) {
        Vec3<f32> co = ray.origin - sphere.center;
        f32 a = ray.direction.dot(ray.direction);
        f32 b = 2 * co.dot(ray.direction);
        f32 c = co.dot(co) - sphere.radius * sphere.radius;
        f32 discriminant = b * b - 4 * a * c;
        if (discriminant < 0) {
            continue;
        }
        f32 t1 = (-b - std::sqrt(discriminant)) / (2 * a);
        if ((t1 >= tMin) && (t1 <= tMax) && (t1 < tClosest)) {
            tClosest = t1;
            closestSphere = sphere;
        }
        f32 t2 = (-b + std::sqrt(discriminant)) / (2 * a);
        if ((t2 >= tMin) && (t2 <= tMax) && (t2 < tClosest)) {
            tClosest = t2;
            closestSphere = sphere;
        }
    }
    if (closestSphere.has_value()) {
        Vec3<f32> point = ray.origin + tClosest * ray.direction;
        Vec3<f32> normal = (point - closestSphere->center).normalize();
        return closestSphere->color
            * compute_lighting(
                point,
                normal,
                -ray.direction,
                closestSphere->shininess,
                lights
            );
    }
    return BLACK;
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
            Vec3<f32> pos = {
                .x = static_cast<f32>(x) * viewport.width
                    / static_cast<f32>(canvas.width()),
                .y = static_cast<f32>(y) * viewport.height
                    / static_cast<f32>(canvas.height()),
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
                    1.0F,
                    std::numeric_limits<f32>::infinity()
                )
            );
        }
    }
}

}