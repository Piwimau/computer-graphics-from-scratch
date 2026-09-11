#include <cmath>
#include <limits>
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
 * @brief Traces a ray through a scene and returns the color of the closest
 * object it intersects with.
 *
 * @param[in] ray     The ray to trace.
 * @param[in] spheres A collection of spheres representing the scene.
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
    f32 tMin,
    f32 tMax
) noexcept {
    f32 tClosest = std::numeric_limits<f32>::infinity();
    Color color = BLACK;
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
            color = sphere.color;
        }
        f32 t2 = (-b + std::sqrt(discriminant)) / (2 * a);
        if ((t2 >= tMin) && (t2 <= tMax) && (t2 < tClosest)) {
            tClosest = t2;
            color = sphere.color;
        }
    }
    return color;
}

void raytrace(
    std::span<const Sphere> spheres,
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
                trace_ray(ray, spheres, 1, std::numeric_limits<f32>::infinity())
            );
        }
    }
}

}