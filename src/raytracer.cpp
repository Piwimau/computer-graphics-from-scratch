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

    /** @brief The origin of this ray. */
    Vec3<f64> origin;

    /** @brief The direction of the ray. */
    Vec3<f64> dir;

    /** @brief The minimum distance to consider for intersections. */
    f64 minDist;

    /** @brief The maximum distance to consider for intersections. */
    f64 maxDist;

};

/** @brief Represents an intersection of a ray with an object. */
struct Hit {

    /** @brief The distance along the ray to the intersection. */
    f64 dist;

    /** @brief The intersection point. */
    Vec3<f64> point;

    /** @brief The surface normal at the intersection point. */
    Vec3<f64> normal;

    /** @brief The material at the intersection point. */
    const Material* material;

};

/** @brief A small offset to avoid self-intersections. */
static constexpr f64 BIAS = 0.001;

/** @brief Represents infinity for floating-point calculations. */
static constexpr f64 INF = std::numeric_limits<f64>::infinity();

/** @brief The epsilon for floating-point calculations. */
static constexpr f64 EPS = std::numeric_limits<f64>::epsilon();

/** @brief The background color used when no objects are intersected. */
static constexpr Color BACKGROUND_COLOR = { 0.0, 0.0, 0.0 };

/** @brief The factor used for gamma correction. */
static constexpr f64 GAMMA = 1.5;

/**
 * @brief Tries to intersect a ray with a sphere.
 *
 * @param[in] sphere The sphere to test for intersection.
 * @param[in] ray    The ray to test for intersection.
 * @return An hit on success, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> intersect(
    const Sphere& sphere,
    const Ray& ray
) noexcept {
    Vec3<f64> co = ray.origin - sphere.center;
    f64 a = ray.dir.dot(ray.dir);
    f64 b = 2.0 * co.dot(ray.dir);
    f64 c = co.dot(co) - sphere.radius * sphere.radius;
    f64 discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return std::nullopt;
    }
    f64 dist0 = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if ((dist0 >= ray.minDist) && (dist0 <= ray.maxDist)) {
        Vec3<f64> point = ray.origin + dist0 * ray.dir;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { dist0, point, normal, &sphere.material };
    }
    f64 dist1 = (-b + std::sqrt(discriminant)) / (2.0 * a);
    if ((dist1 >= ray.minDist) && (dist1 <= ray.maxDist)) {
        Vec3<f64> point = ray.origin + dist1 * ray.dir;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { dist1, point, normal, &sphere.material };
    }
    return std::nullopt;
}

/**
 * @brief Tries to intersect a ray with an axis-aligned bounding box.
 *
 * @param[in] bounds The axis-aligned bounding box to test for intersection.
 * @param[in] ray    The ray to test for intersection.
 * @return `true` if an intersection is found, otherwise `false`.
 */
static constexpr bool intersect(const Aabb& bounds, const Ray& ray) noexcept {
    f64 minDist = ray.minDist;
    f64 maxDist = ray.maxDist;
    auto slab = [&](f64 min, f64 max, f64 origin, f64 dir) -> bool {
        f64 invDir = 1.0 / dir;
        f64 dist0 = (min - origin) * invDir;
        f64 dist1 = (max - origin) * invDir;
        if (invDir < 0.0) {
            std::swap(dist0, dist1);
        }
        minDist = std::max(minDist, dist0);
        maxDist = std::min(maxDist, dist1);
        return maxDist > minDist;
    };
    return slab(bounds.min.x, bounds.max.x, ray.origin.x, ray.dir.x)
        && slab(bounds.min.y, bounds.max.y, ray.origin.y, ray.dir.y)
        && slab(bounds.min.z, bounds.max.z, ray.origin.z, ray.dir.z);
}

/**
 * @brief Tries to intersect a ray with a mesh.
 *
 * @param[in] mesh The mesh to test for intersection.
 * @param[in] ray  The ray to test for intersection.
 * @return A hit on success, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> intersect(
    const Mesh& mesh,
    const Ray& ray
) noexcept {
    std::optional<Hit> closestHit;
    if (!intersect(mesh.bounds, ray)) {
        return closestHit;
    }
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
        f64 dist = e1.dot(c) * invDet;
        if (
            (dist >= ray.minDist) && (dist <= ray.maxDist)
                && (!closestHit || (dist < closestHit->dist))
        ) {
            closestHit = {
                .dist = dist,
                .point = ray.origin + dist * ray.dir,
                .normal = e0.cross(e1).normalize(),
                .material = &mesh.material
            };
        }
    }
    return closestHit;
}

/**
 * @brief Finds the closest intersection of a ray with a collection of objects.
 *
 * @param[in] objects A collection of objects to test for intersections.
 * @param[in] ray     The ray to test for intersections.
 * @return A hit on success, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> closest_intersection(
    std::span<const Object> objects,
    const Ray& ray
) noexcept {
    std::optional<Hit> closestHit;
    for (const Object& object : objects) {
        std::visit(
            [&]<typename T>(const T& o) {
                std::optional<Hit> hit = intersect(
                    o,
                    {
                        .origin = ray.origin,
                        .dir = ray.dir,
                        .minDist = ray.minDist,
                        .maxDist = closestHit ? closestHit->dist : ray.maxDist
                    }
                );
                if (hit && (!closestHit || (hit->dist < closestHit->dist))) {
                    closestHit = hit;
                }
            },
            object
        );
    }
    return closestHit;
}

/**
 * @brief Tries to intersect a ray with a collection of objects.
 *
 * @param[in] objects A collection of objects to test for intersections.
 * @param[in] ray     The ray to test for intersections.
 * @return `true` if the ray intersects any object, otherwise `false`.
 */
static constexpr bool any_intersection(
    std::span<const Object> objects,
    const Ray& ray
) noexcept {
    for (const Object& object : objects) {
        bool hit = std::visit(
            [&]<typename T>(const T& o) {
                return intersect(o, ray) != std::nullopt;
            },
            object
        );
        if (hit) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Computes the local color at a point on a surface, accounting for the
 * material properties, lights, and other objects in the scene.
 *
 * @param[in] point    The point to compute the local color for.
 * @param[in] normal   The surface normal at that point (normalized).
 * @param[in] viewDir  The direction from the point towards the camera
 *                     (normalized).
 * @param[in] material The material of the surface at the point.
 * @param[in] scene    The scene containing objects and lights.
 * @return The local color at the point.
 */
static constexpr Color local_color(
    const Vec3<f64>& point,
    const Vec3<f64>& normal,
    const Vec3<f64>& viewDir,
    const Material& material,
    const Scene& scene
) noexcept {
    auto is_blocked = [&](const Vec3<f64>& lightDir, f64 maxDist) -> bool {
        return any_intersection(
            scene.objects,
            {
                .origin = point,
                .dir = lightDir,
                .minDist = BIAS,
                .maxDist = maxDist
            }
        );
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
        f64 dist = lightDir.norm();
        lightDir = lightDir.normalize();
        if (!is_blocked(lightDir, dist)) {
            f64 attenuation = 1.0
                / (light.kc + light.kl * dist + light.kq * dist * dist);
            local += material.diffuse * light.color * light.intensity
                * diff(lightDir) * attenuation;
            if (material.shininess > 0.0) {
                local += material.specular * light.color * light.intensity
                    * spec(lightDir) * attenuation;
            }
        }
    }
    for (const SpotLight& light : scene.spotLights) {
        Vec3<f64> lightDir = light.pos - point;
        f64 dist = lightDir.norm();
        lightDir = lightDir.normalize();
        if (!is_blocked(lightDir, dist)) {
            f64 cosTheta = (-lightDir).dot(light.dir);
            if (cosTheta <= light.outerCutoff) {
                continue;
            }
            f64 spot = std::clamp(
                (cosTheta - light.outerCutoff)
                    / (light.innerCutoff - light.outerCutoff),
                0.0,
                1.0
            );
            f64 attenuation = spot
                / (light.kc + light.kl * dist + light.kq * dist * dist);
            local += material.diffuse * light.color * light.intensity
                * diff(lightDir) * attenuation;
            if (material.shininess > 0.0) {
                local += material.specular * light.color * light.intensity
                    * spec(lightDir) * attenuation;
            }
        }
    }
    for (const DirectionalLight& light : scene.directionalLights) {
        Vec3<f64> lightDir = (-light.dir).normalize();
        f64 maxDist = INF;
        if (!is_blocked(lightDir, maxDist)) {
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
 * object it intersects with.
 *
 * @param[in] scene The scene containing objects and lights.
 * @param[in] ray   The ray to trace.
 * @param[in] depth The maximum recursion depth.
 * @return The color of the closest object the ray intersects with, or
 * `BACKGROUND_COLOR` if no object is intersected.
 */
static constexpr Color trace_ray(
    const Scene& scene,
    const Ray& ray,
    isize depth = 8
) noexcept {
    assert(depth >= 0);
    Color color = BACKGROUND_COLOR;
    std::optional<Hit> hit = closest_intersection(scene.objects, ray);
    if (!hit) {
        return color;
    }
    color = local_color(
        hit->point,
        hit->normal,
        (-ray.dir).normalize(),
        *hit->material,
        scene
    );
    if ((depth == 0) || (hit->material->reflectivity <= 0.0)) {
        return color;
    }
    Color reflected = trace_ray(
        scene,
        {
            .origin = hit->point,
            .dir = (-ray.dir).reflect(hit->normal).normalize(),
            .minDist = BIAS,
            .maxDist = INF
        },
        depth - 1
    );
    color = Color::lerp(color, reflected, hit->material->reflectivity);
    return color;
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

void raytrace(const Scene& scene, Canvas& canvas, isize samples) {
    assert(samples > 0);
    Vec2<f64> canvasSize = {
        .x = static_cast<f64>(canvas.width()),
        .y = static_cast<f64>(canvas.height())
    };
    Vec2<f64> viewportScale = {
        .x = scene.viewport.width / canvasSize.x,
        .y = scene.viewport.height / canvasSize.y
    };
    for (isize y = 0; y < canvas.height(); y++) {
        for (isize x = 0; x < canvas.width(); x++) {
            Color avg = { };
            for (isize sy = 0; sy < samples; sy++) {
                for (isize sx = 0; sx < samples; sx++) {
                    Vec3<f64> viewportPos = {
                        .x = (-canvasSize.x / 2.0 + static_cast<f64>(x)
                              + (static_cast<f64>(sx) + 0.5)
                                  / static_cast<f64>(samples))
                            * viewportScale.x,
                        .y = (canvasSize.y / 2.0 - 1.0 - static_cast<f64>(y)
                              - (static_cast<f64>(sy) + 0.5)
                                  / static_cast<f64>(samples))
                            * viewportScale.y,
                        .z = -scene.viewport.distance
                    };
                    Ray ray = {
                        .origin = scene.camera.pos,
                        .dir = (scene.camera.rot * viewportPos).normalize(),
                        .minDist = scene.viewport.distance,
                        .maxDist = INF
                    };
                    avg += trace_ray(scene, ray)
                        * (1.0 / static_cast<f64>(samples * samples));
                }
            }
            Color color = gamma_correct(avg);
            Rgb rgb = {
                .r = static_cast<u8>(color.r * 255.0 + 0.5),
                .g = static_cast<u8>(color.g * 255.0 + 0.5),
                .b = static_cast<u8>(color.b * 255.0 + 0.5)
            };
            canvas.draw_pixel({ x, y }, rgb);
        }
    }
}

}