#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include "cgfs/raytracer.hpp"

namespace cgfs {

/** @brief Represents a ray. */
struct Ray {

    /** @brief The origin of the ray. */
    Vec3<f64> origin;

    /** @brief The direction of the ray. */
    Vec3<f64> dir;

};

/** @brief Represents an interval. */
struct Interval {

    /** @brief The lower bound of this interval. */
    f64 min;

    /** @brief The upper bound of this interval (inclusive). */
    f64 max;

};

/** @brief Represents a hit of a ray with an object. */
struct Hit {

    /** @brief The distance along the ray to the hit point. */
    f64 t;

    /** @brief The hit point. */
    Vec3<f64> point;

    /** @brief The surface normal at the hit point. */
    Vec3<f64> normal;

    /** @brief The material at the hit point. */
    const Material* material;

};

/** @brief A small offset to avoid self-intersections. */
static constexpr f64 BIAS = 0.001;

/** @brief Represents infinity for floating-point calculations. */
static constexpr f64 INF = std::numeric_limits<f64>::infinity();

/** @brief The epsilon for floating-point calculations. */
static constexpr f64 EPS = std::numeric_limits<f64>::epsilon();

/** @brief The background color used when no objects are hit. */
static constexpr Color BACKGROUND_COLOR = { 0.0, 0.0, 0.0 };

/** @brief The factor used for gamma correction. */
static constexpr f64 GAMMA = 1.5;

/**
 * @brief Converts canvas coordinates to viewport coordinates.
 *
 * @param[in] canvasPos  The position on the canvas.
 * @param[in] viewport   The viewport to map the canvas coordinates to.
 * @param[in] canvasSize The size of the canvas.
 * @return The corresponding coordinates on the viewport.
 */
static constexpr Vec3<f64> canvas_to_viewport(
    const Vec2<isize>& canvasPos,
    const Viewport& viewport,
    const Vec2<isize>& canvasSize
) noexcept {
    return {
        .x = (static_cast<f64>(canvasPos.x) + 0.5) * viewport.width
            / static_cast<f64>(canvasSize.x),
        .y = (static_cast<f64>(canvasPos.y) + 0.5) * viewport.height
            / static_cast<f64>(canvasSize.y),
        .z = -viewport.distance
    };
}

/**
 * @brief Computes the intersection of a ray with a sphere.
 *
 * @param[in] sphere The sphere to test for intersection.
 * @param[in] ray    The ray to test for intersection.
 * @param[in] t      The interval along the ray to consider for intersections.
 * @return A hit if an intersection is found, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> intersect(
    const Sphere& sphere,
    const Ray& ray,
    Interval t
) noexcept {
    Vec3<f64> co = ray.origin - sphere.center;
    f64 a = ray.dir.dot(ray.dir);
    f64 b = 2.0 * co.dot(ray.dir);
    f64 c = co.dot(co) - sphere.radius * sphere.radius;
    f64 discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return std::nullopt;
    }
    f64 t1 = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if ((t1 >= t.min) && (t1 <= t.max)) {
        Vec3<f64> point = ray.origin + t1 * ray.dir;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { t1, point, normal, &sphere.material };
    }
    f64 t2 = (-b + std::sqrt(discriminant)) / (2.0 * a);
    if ((t2 >= t.min) && (t2 <= t.max)) {
        Vec3<f64> point = ray.origin + t2 * ray.dir;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { t2, point, normal, &sphere.material };
    }
    return std::nullopt;
}

/**
 * @brief Computes the intersection of a ray with a mesh.
 *
 * @param[in] mesh The mesh to test for intersection.
 * @param[in] ray  The ray to test for intersection.
 * @param[in] t    The interval along the ray to consider for intersections.
 * @return A hit if an intersection is found, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> intersect(
    const Mesh& mesh,
    const Ray& ray,
    Interval t
) noexcept {
    std::optional<Hit> closestHit;
    f64 tClosest = INF;
    for (const auto& indices : std::views::chunk(mesh.indices, 3)) {
        Vec3<f64> v0 = mesh.vertices[indices[0]];
        Vec3<f64> v1 = mesh.vertices[indices[1]];
        Vec3<f64> v2 = mesh.vertices[indices[2]];
        Vec3<f64> e0 = v1 - v0;
        Vec3<f64> e1 = v2 - v0;
        Vec3<f64> a = ray.dir.cross(e1);
        f64 det = e0.dot(a);
        if (std::abs(det) < EPS) {
            continue;
        }
        f64 invDet = 1.0 / det;
        Vec3<f64> b = ray.origin - v0;
        f64 u = b.dot(a) * invDet;
        if ((u < 0.0) || (u > 1.0)) {
            continue;
        }
        Vec3<f64> c = b.cross(e0);
        f64 v = ray.dir.dot(c) * invDet;
        if ((v < 0.0) || (u + v > 1.0)) {
            continue;
        }
        f64 tHit = e1.dot(c) * invDet;
        if (
            (tHit >= t.min) && (tHit <= t.max)
                && (!closestHit || (tHit < tClosest))
        ) {
            closestHit = {
                .t = tHit,
                .point = ray.origin + tHit * ray.dir,
                .normal = e0.cross(e1).normalize(),
                .material = &mesh.material
            };
            tClosest = tHit;
        }
    }
    return closestHit;
}

/**
 * @brief Finds the closest intersection of a ray with a collection of objects.
 *
 * @param[in] objects A collection of objects to test for intersections.
 * @param[in] ray     The ray to test for intersections.
 * @param[in] t       The interval along the ray to consider for intersections.
 * @return A hit containing the index of the closest intersected object and the
 * distance along the ray, or `std::nullopt` if no intersection is found.
 */
static constexpr std::optional<Hit> closest_intersection(
    std::span<const Object> objects,
    const Ray& ray,
    Interval t
) noexcept {
    std::optional<Hit> closestHit;
    f64 tClosest = INF;
    for (const Object& object : objects) {
        std::visit(
            [&]<typename T>(const T& o) {
                std::optional<Hit> hit = intersect(
                    o,
                    ray,
                    { .min = t.min, .max = closestHit ? tClosest : t.max }
                );
                if (hit && (!closestHit || (hit->t < tClosest))) {
                    closestHit = hit;
                    tClosest = hit->t;
                }
            },
            object
        );
    }
    return closestHit;
}

/**
 * @brief Computes the shading at a point on a surface, accounting for the
 * material properties, lights, and other objects in the scene.
 *
 * @param[in] point    The point to compute the shading for.
 * @param[in] normal   The surface normal at that point (normalized).
 * @param[in] viewDir  The direction from the point towards the camera
 *                     (normalized).
 * @param[in] material The material of the surface at the point.
 * @param[in] scene    The scene containing the objects and lights.
 * @return The shading at the specified point.
 */
static constexpr Color compute_shading(
    const Vec3<f64>& point,
    const Vec3<f64>& normal,
    const Vec3<f64>& viewDir,
    const Material& material,
    const Scene& scene
) noexcept {
    auto is_blocked = [&](const Vec3<f64>& lightDir, f64 tMax) -> bool {
        std::optional<Hit> hit = closest_intersection(
            scene.objects,
            { .origin = point, .dir = lightDir },
            { .min = BIAS, .max = tMax }
        );
        return hit != std::nullopt;
    };
    auto diff = [&](const Vec3<f64>& lightDir) -> f64 {
        return std::max(normal.dot(lightDir), 0.0);
    };
    auto spec = [&](const Vec3<f64>& lightDir) -> f64 {
        Vec3<f64> halfDir = (lightDir + viewDir).normalize();
        return std::pow(std::max(normal.dot(halfDir), 0.0), material.shininess);
    };
    Color local = material.ambient * scene.ambientLight.color
        * scene.ambientLight.intensity;
    for (const PointLight& light : scene.pointLights) {
        Vec3<f64> lightDir = light.pos - point;
        f64 tMax = lightDir.norm();
        lightDir = lightDir.normalize();
        if (!is_blocked(lightDir, tMax)) {
            local += material.diffuse * light.color * light.intensity
                * diff(lightDir);
            if (material.shininess > 0.0) {
                local += material.specular * light.color * light.intensity
                    * spec(lightDir);
            }
        }
    }
    for (const DirectionalLight& light : scene.directionalLights) {
        Vec3<f64> lightDir = (-light.dir).normalize();
        f64 tMax = INF;
        if (!is_blocked(lightDir, tMax)) {
            local += material.diffuse * light.color * light.intensity
                * diff(lightDir);
            if (material.shininess > 0.0) {
                local += material.specular * light.color * light.intensity
                    * spec(lightDir);
            }
        }
    }
    return local;
}

/**
 * @brief Traces a ray through a scene and returns the color of the closest
 * object it intersects with (if any).
 *
 * @param[in] scene The scene containing objects and lights.
 * @param[in] ray   The ray to trace.
 * @param[in] t     The interval along the ray to consider for intersections.
 * @param[in] depth The maximum recursion depth.
 * @return The color of the closest object the ray intersects with, or
 * `BACKGROUND_COLOR` if it does not intersect with any objects.
 */
static Color trace_ray(
    const Scene& scene,
    const Ray& ray,
    Interval t,
    isize depth = 8
) noexcept {
    assert(depth >= 0);
    std::optional<Hit> hit = closest_intersection(scene.objects, ray, t);
    if (!hit) {
        return BACKGROUND_COLOR;
    }
    if ((depth == 0) || (hit->material->reflectivity <= 0.0)) {
        return compute_shading(
            hit->point,
            hit->normal,
            (-ray.dir).normalize(),
            *hit->material,
            scene
        );
    }
    Color local = compute_shading(
        hit->point,
        hit->normal,
        (-ray.dir).normalize(),
        *hit->material,
        scene
    );
    Color reflected = trace_ray(
        scene,
        { .origin = hit->point, .dir = (-ray.dir).reflect(hit->normal) },
        { .min = BIAS, .max = INF },
        depth - 1
    );
    return Color::lerp(local, reflected, hit->material->reflectivity);
}

/**
 * @brief Applies gamma correction to a color.
 *
 * @param[in] color The color to correct.
 * @return The gamma-corrected color.
 */
static constexpr Color gamma_correct(const Color& color) noexcept {
    return {
        .r = std::pow(color.r, 1.0 / GAMMA),
        .g = std::pow(color.g, 1.0 / GAMMA),
        .b = std::pow(color.b, 1.0 / GAMMA)
    };
}

void raytrace(const Scene& scene, Canvas& canvas) {
    for (isize y = canvas.max_y(); y >= canvas.min_y(); y--) {
        for (isize x = canvas.min_x(); x <= canvas.max_x(); x++) {
            Vec2<isize> canvasPos = { x, y };
            Vec3<f64> viewportPos = canvas_to_viewport(
                canvasPos,
                scene.viewport,
                { canvas.width(), canvas.height() }
            );
            Ray ray = {
                .origin = scene.camera.pos,
                .dir = scene.camera.rot * viewportPos
            };
            Color color = gamma_correct(
                trace_ray(scene, ray, { .min = 1.0, .max = INF })
            );
            canvas.draw_pixel(
                canvasPos,
                {
                    .r = static_cast<u8>(color.r * 255.0 + 0.5),
                    .g = static_cast<u8>(color.g * 255.0 + 0.5),
                    .b = static_cast<u8>(color.b * 255.0 + 0.5)
                }
            );
        }
    }
}

}