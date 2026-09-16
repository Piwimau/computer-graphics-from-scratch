#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <ranges>
#include "cgfs/raytracer.hpp"
#include "cgfs/vec2.hpp"

namespace cgfs {

/** @brief Represents a ray. */
struct Ray {

    /** @brief The origin of this ray. */
    Vec3<f64> origin;

    /** @brief The direction of the ray. */
    Vec3<f64> direction;

    /** @brief The minimum distance to consider for intersections. */
    f64 tMin;

    /** @brief The maximum distance to consider for intersections. */
    f64 tMax;

};

/** @brief Represents an intersection of a ray with an object. */
struct Hit {

    /** @brief The distance along the ray to the intersection point. */
    f64 t;

    /** @brief The intersection point. */
    Vec3<f64> point;

    /** @brief The surface normal at the intersection point. */
    Vec3<f64> normal;

    /** @brief The material at the intersection point. */
    Material material;

};

/** @brief The value of pi. */
static constexpr f64 PI = std::numbers::pi_v<f64>;

/** @brief Represents infinity for floating-point calculations. */
static constexpr f64 INF = std::numeric_limits<f64>::infinity();

/** @brief The epsilon for floating-point calculations. */
static constexpr f64 EPS = std::numeric_limits<f64>::epsilon();

/** @brief The background color used when no objects are intersected. */
static constexpr Color BACKGROUND_COLOR = BLACK;

/** @brief A small offset used to avoid self-intersections. */
static constexpr f64 DISTANCE_OFFSET = 0.001;

/** @brief The minimum roughness to avoid artifacts. */
static constexpr f64 MIN_ROUGHNESS = 0.001;

/** @brief The falloff coefficient for reflections. */
static constexpr f64 REFLECTION_FALLOFF = 2.0;

/** @brief The factor used for gamma correction. */
static constexpr f64 GAMMA = 2.2;

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
    f64 a = ray.direction.dot(ray.direction);
    f64 b = 2.0 * co.dot(ray.direction);
    f64 c = co.dot(co) - sphere.radius * sphere.radius;
    f64 discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return std::nullopt;
    }
    f64 t0 = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if ((t0 >= ray.tMin) && (t0 <= ray.tMax)) {
        Vec3<f64> point = ray.origin + t0 * ray.direction;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { t0, point, normal, sphere.material };
    }
    f64 t1 = (-b + std::sqrt(discriminant)) / (2.0 * a);
    if ((t1 >= ray.tMin) && (t1 <= ray.tMax)) {
        Vec3<f64> point = ray.origin + t1 * ray.direction;
        Vec3<f64> normal = (point - sphere.center).normalize();
        return Hit { t1, point, normal, sphere.material };
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
    f64 tMin = ray.tMin;
    f64 tMax = ray.tMax;
    auto slab = [&](f64 min, f64 max, f64 origin, f64 dir) -> bool {
        f64 invDir = 1.0 / dir;
        f64 t0 = (min - origin) * invDir;
        f64 t1 = (max - origin) * invDir;
        if (invDir < 0.0) {
            std::swap(t0, t1);
        }
        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);
        return tMax > tMin;
    };
    return slab(bounds.min.x, bounds.max.x, ray.origin.x, ray.direction.x)
        && slab(bounds.min.y, bounds.max.y, ray.origin.y, ray.direction.y)
        && slab(bounds.min.z, bounds.max.z, ray.origin.z, ray.direction.z);
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
    for (const auto& [v0, v1, v2] : mesh.triangles()) {
        Vec3<f64> e0 = v1 - v0;
        Vec3<f64> e1 = v2 - v0;
        Vec3<f64> a = ray.direction.cross(e1);
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
        f64 v = ray.direction.dot(c) * invDet;
        if ((v < 0.0) || (u + v > 1.0)) {
            continue;
        }
        f64 t = e1.dot(c) * invDet;
        if (
            (t >= ray.tMin) && (t <= ray.tMax)
                && (!closestHit || (t < closestHit->t))
        ) {
            closestHit = {
                .t = t,
                .point = ray.origin + t * ray.direction,
                .normal = e0.cross(e1).normalize(),
                .material = mesh.material
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
                        .direction = ray.direction,
                        .tMin = ray.tMin,
                        .tMax = closestHit ? closestHit->t : ray.tMax
                    }
                );
                if (hit && (!closestHit || (hit->t < closestHit->t))) {
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
 * @brief Computes the Fresnel effect using the Schlick approximation.
 *
 * @param[in] cosTheta The cosine of the angle between the view direction and
 *                     the surface normal.
 * @param[in] f0       The reflectance color at normal incidence.
 * @return The Fresnel reflectance color.
 */
static constexpr Color fresnel_schlick(f64 cosTheta, Color f0) noexcept {
    f64 t = std::pow(1.0 - std::clamp(cosTheta, 0.0, 1.0), 5.0);
    return Color::lerp(f0, WHITE, t);
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
    auto is_blocked = [&](const Vec3<f64>& lightDir, f64 tMax) -> bool {
        return any_intersection(
            scene.objects,
            {
                .origin = point,
                .direction = lightDir,
                .tMin = DISTANCE_OFFSET,
                .tMax = tMax
            }
        );
    };
    auto lighting = [&](const Vec3<f64>& lightDir) -> Color {
        Vec3<f64> halfDir = (lightDir + viewDir).normalize();
        f64 vh = std::max(viewDir.dot(halfDir), 0.0);
        Color fresnel = fresnel_schlick(vh, material.f0);
        Color kd = (WHITE - fresnel) * (1.0 - material.metalness);
        f64 nl = std::max(normal.dot(lightDir), 0.0);
        Color diffuse = kd * material.albedo * nl;
        f64 shininess = 2.0
                / std::pow(std::max(material.roughness, MIN_ROUGHNESS), 4)
            - 2.0;
        f64 norm = (shininess + 8.0) / (8.0 * PI);
        f64 nh = std::max(normal.dot(halfDir), 0.0);
        Color specular = fresnel * norm * std::pow(nh, shininess);
        return diffuse + specular;
    };
    Color local = BLACK;
    for (const PointLight& light : scene.pointLights) {
        Vec3<f64> lightDir = light.position - point;
        f64 distance = lightDir.norm();
        lightDir = lightDir.normalize();
        if (!is_blocked(lightDir, distance)) {
            f64 attenuation = 1.0
                / (light.kc + light.kl * distance
                   + light.kq * distance * distance);
            local += lighting(lightDir) * light.color * light.intensity
                * attenuation;
        }
    }
    for (const SpotLight& light : scene.spotLights) {
        Vec3<f64> lightDir = light.position - point;
        f64 distance = lightDir.norm();
        lightDir = lightDir.normalize();
        if (!is_blocked(lightDir, distance)) {
            f64 cosTheta = (-lightDir).dot(light.direction);
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
                / (light.kc + light.kl * distance
                   + light.kq * distance * distance);
            local += lighting(lightDir) * light.color * light.intensity
                * attenuation;
        }
    }
    for (const DirectionalLight& light : scene.directionalLights) {
        Vec3<f64> lightDir = (-light.direction).normalize();
        if (!is_blocked(lightDir, INF)) {
            local += lighting(lightDir) * light.color * light.intensity;
        }
    }
    return local;
}

/**
 * @brief Traces a ray through a scene and returns the surface color of the
 * closest object it intersects with.
 *
 * @param[in] scene The scene containing objects and lights.
 * @param[in] ray   The ray to trace.
 * @param[in] depth The maximum recursion depth.
 * @return The surface color of the closest object the ray intersects with, or
 * `BACKGROUND_COLOR` if no intersection is found.
 */
static constexpr Color trace_ray(
    const Scene& scene,
    const Ray& ray,
    isize depth = 3
) noexcept {
    assert(depth >= 0);
    Color result = BACKGROUND_COLOR;
    std::optional<Hit> hit = closest_intersection(scene.objects, ray);
    if (!hit) {
        return result;
    }
    const auto& [_, point, normal, material] = *hit;
    Vec3<f64> viewDir = (-ray.direction).normalize();
    Color local = local_color(point, normal, viewDir, material, scene);
    if (depth > 0) {
        f64 nv = std::max(normal.dot(viewDir), 0.0);
        Color reflectivity = fresnel_schlick(nv, material.f0)
            * std::pow(1.0 - material.roughness, REFLECTION_FALLOFF);
        Ray reflectedRay = {
            .origin = point,
            .direction = viewDir.reflect(normal),
            .tMin = DISTANCE_OFFSET,
            .tMax = INF
        };
        Color reflected = trace_ray(scene, reflectedRay, depth - 1);
        result = Color::lerp(local, reflected, reflectivity);
    }
    return result;
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
                        .origin = scene.camera.viewpoint,
                        .direction = (scene.camera.rotation * viewportPos)
                            .normalize(),
                        .tMin = scene.viewport.distance,
                        .tMax = INF
                    };
                    avg += trace_ray(scene, ray)
                        / static_cast<f64>(samples * samples);
                }
            }
            Color color = gamma_correct(avg).clamp(0.0, 1.0);
            Pixel pixel = {
                .r = static_cast<u8>(color.r * 255.0 + 0.5),
                .g = static_cast<u8>(color.g * 255.0 + 0.5),
                .b = static_cast<u8>(color.b * 255.0 + 0.5)
            };
            canvas.put_pixel(x, y, pixel);
        }
    }
}

}