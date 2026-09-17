#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <ranges>
#include <thread>
#include <utility>
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

/** @brief The maximum recursion depth for ray tracing. */
static constexpr isize MAX_TRACE_DEPTH = 3;

/**
 * @brief Represents a simple stack for tracking refractive indices of
 * transparent media.
 */
class IorStack final {
private:

    /** @brief The stack of refractive indices. */
    std::array<f64, MAX_TRACE_DEPTH + 1> _iors;

    /** @brief The current size of the stack. */
    isize _size;

public:

    /** @brief Initializes an new stack of refractive indices. */
    constexpr IorStack() noexcept : _iors({ }), _size(0) { }

    /**
     * @brief Returns the refractive index of the current medium, or `1.0` if
     * this stack is empty.
     *
     * @return The refractive index of the current medium.
     */
    constexpr f64 current() const noexcept {
        return (_size > 0) ? _iors[_size - 1] : 1.0;
    }

    /**
     * @brief Returns the refractive index of the previous medium, or `1.0` if
     * this stack has fewer than two elements.
     *
     * @return The refractive index of the previous medium.
     */
    constexpr f64 previous() const noexcept {
        return (_size > 1) ? _iors[_size - 2] : 1.0;
    }

    /**
     * @brief Enters a new medium by pushing its refractive index onto this
     * stack.
     *
     * @warning The behavior is undefined if this stack is full, or if `ior` is
     * not greater than zero.
     *
     * @param[in] ior The refractive index of the new medium.
     */
    constexpr void push(f64 ior) noexcept {
        assert(_size < std::ssize(_iors));
        assert(ior > 0.0);
        _iors[_size++] = ior;
    }

    /**
     * @brief Leaves the current medium by popping its refractive index from
     * this stack.
     *
     * @warning The behavior is undefined if this stack is empty.
     */
    constexpr void pop() noexcept {
        assert(_size > 0);
        _size--;
    }

};

/** @brief Represents a helper to allow overloading of lambdas. */
template<typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

/** @brief A deduction guide for the overload helper. */
template<typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

/** @brief The value of pi. */
static constexpr f64 PI = std::numbers::pi_v<f64>;

/** @brief Represents infinity for floating-point calculations. */
static constexpr f64 INF = std::numeric_limits<f64>::infinity();

/** @brief The epsilon for floating-point calculations. */
static constexpr f64 EPS = 1.0E-6;

/** @brief The background color used when no objects are intersected. */
static constexpr Color BACKGROUND_COLOR = { 0.0, 0.0, 0.0 };

/** @brief The maximum depth for transparent occluders. */
static constexpr isize MAX_TRANSPARENCY_DEPTH = 8;

/** @brief The factor used for gamma correction. */
static constexpr f64 GAMMA = 2.2;

/** @brief The maximum number of threads to use for rendering. */
static const usize MAX_THREADS = std::max<usize>(
    std::thread::hardware_concurrency(),
    1
);

/**
 * @brief Tries to intersect a ray with a sphere.
 *
 * @param[in] sphere The sphere to test for intersection.
 * @param[in] ray    The ray to test for intersection.
 * @return An hit on success, otherwise `std::nullopt`.
 */
static constexpr std::optional<Hit> intersect_sphere(
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
static constexpr bool intersect_aabb(
    const Aabb& bounds,
    const Ray& ray
) noexcept {
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
static constexpr std::optional<Hit> intersect_mesh(
    const Mesh& mesh,
    const Ray& ray
) noexcept {
    std::optional<Hit> closestHit;
    if (mesh.bounds && !intersect_aabb(*mesh.bounds, ray)) {
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
        Ray updatedRay = {
            .origin = ray.origin,
            .direction = ray.direction,
            .tMin = ray.tMin,
            .tMax = closestHit ? closestHit->t : ray.tMax
        };
        std::optional<Hit> hit = std::visit(
            Overloaded {
                [&](const Sphere& sphere) {
                    return intersect_sphere(sphere, updatedRay);
                },
                [&](const Mesh& mesh) {
                    return intersect_mesh(mesh, updatedRay);
                }
            },
            object
        );
        if (hit && (!closestHit || (hit->t < closestHit->t))) {
            closestHit = hit;
        }
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
static constexpr bool intersect_any(
    std::span<const Object> objects,
    const Ray& ray
) noexcept {
    for (const Object& object : objects) {
        std::optional<Hit> hit = std::visit(
            Overloaded {
                [&](const Sphere& sphere) {
                    return intersect_sphere(sphere, ray);
                },
                [&](const Mesh& mesh) {
                    return intersect_mesh(mesh, ray);
                }
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
 * @brief Offsets a ray origin to avoid self-intersections.
 *
 * @param[in] point  The original point of intersection.
 * @param[in] normal The normal at the intersection point.
 * @return The offset ray origin.
 */
static constexpr Vec3<f64> offset_ray_origin(
    const Vec3<f64>& point,
    const Vec3<f64>& normal
) noexcept {
    constexpr f64 ORIGIN = 1.0 / 32.0;
    constexpr f64 FLOAT_SCALE = 1.0 / 65536.0;
    constexpr f64 INT_SCALE = 256.0;
    auto offset = [](f64 p, f64 n) -> f64 {
        i64 i = static_cast<i64>(INT_SCALE * n);
        i64 bits = std::bit_cast<i64>(p);
        f64 shifted = std::bit_cast<f64>(bits + ((p < 0.0) ? -i : i));
        return (std::abs(p) < ORIGIN) ? p + FLOAT_SCALE * n : shifted;
    };
    return {
        offset(point.x, normal.x),
        offset(point.y, normal.y),
        offset(point.z, normal.z)
    };
}

/**
 * @brief Computes an orthonormal basis around a normal.
 *
 * @param[in] normal A normal around which to compute the basis (normalized).
 * @return A pair containing the tangent and bitangent forming an orthonormal
 * basis with the normal.
 */
static constexpr std::pair<Vec3<f64>, Vec3<f64>> orthonormal_basis(
    const Vec3<f64>& normal
) noexcept {
    f64 sign = std::copysign(1.0, normal.z);
    f64 a = -1.0 / (sign + normal.z);
    f64 b = normal.x * normal.y * a;
    return {
        { 1.0 + sign * normal.x * normal.x * a, sign * b, -sign * normal.x },
        { b, sign + normal.y * normal.y * a, -normal.y }
    };
}

/**
 * @brief Uniformly samples a point on a unit disk.
 *
 * @param[in, out] rng The random engine to draw from.
 * @return A point on the unit disk.
 */
static Vec2<f64> sample_disk(std::mt19937& rng) noexcept {
    std::uniform_real_distribution<f64> dist(-1.0, 1.0);
    f64 r = std::sqrt(dist(rng) * 0.5 + 0.5);
    f64 theta = (dist(rng) * 0.5 + 0.5) * 2.0 * PI;
    return { r * std::cos(theta), r * std::sin(theta) };
}

/**
 * @brief Computes the Fresnel reflectance using the Schlick approximation.
 *
 * @param[in] cosTheta The cosine of the angle between the view/light direction
 *                     and the half-vector (or normal).
 * @param[in] f0       The reflectance at normal incidence.
 * @return The Fresnel reflectance.
 */
static constexpr Color fresnel_schlick(f64 cosTheta, const Color& f0) noexcept {
    f64 x = 1.0 - std::clamp(cosTheta, 0.0, 1.0);
    f64 t = x * x * x * x * x;
    return Color::lerp(f0, { 1.0, 1.0, 1.0 }, t);
}

/**
 * @brief Computes the local color at a point on a surface, accounting for the
 * material properties, lights, and other objects in the scene.
 *
 * @param[in]      point    The point to compute the local color for.
 * @param[in]      normal   The surface normal at that point (normalized).
 * @param[in]      viewDir  The direction from the point towards the camera
 *                          (normalized).
 * @param[in]      material The material of the surface at the point.
 * @param[in]      scene    The scene containing objects and lights.
 * @param[in, out] rng      The random engine to draw from.
 * @return The local color at the point.
 */
static Color local_color(
    const Vec3<f64>& point,
    const Vec3<f64>& normal,
    const Vec3<f64>& viewDir,
    const Material& material,
    const Scene& scene,
    std::mt19937& rng
) noexcept {
    auto shadow_attenuation = [&](
        const Vec3<f64>& lightDir,
        f64 tMax
    ) -> Color {
        Color attenuation = { 1.0, 1.0, 1.0 };
        Vec3<f64> origin = offset_ray_origin(point, normal);
        for (isize i = 0; i < MAX_TRANSPARENCY_DEPTH; i++) {
            std::optional<Hit> hit = closest_intersection(
                scene.objects,
                {
                    .origin = origin,
                    .direction = lightDir,
                    .tMin = 0.0,
                    .tMax = tMax
                }
            );
            if (!hit) {
                break;
            }
            if (!hit->material.is_transparent()) {
                attenuation = { 0.0, 0.0, 0.0 };
                break;
            }
            attenuation *= hit->material.albedo * hit->material.transparency
                + Color { 1.0, 1.0, 1.0 } * (1.0 - hit->material.transparency);
            origin = offset_ray_origin(hit->point, hit->normal);
            tMax -= hit->t;
        }
        return attenuation;
    };
    f64 alpha = std::max(material.roughness * material.roughness, EPS);
    f64 alpha2 = alpha * alpha;
    f64 nDotV = std::max(normal.dot(viewDir), 0.0);
    f64 k = (material.roughness + 1.0) * (material.roughness + 1.0) / 8.0;
    f64 geometryV = nDotV / (nDotV * (1.0 - k) + k);
    auto lighting = [&](const Vec3<f64>& lightDir) -> Color {
        Vec3<f64> halfDir = (viewDir + lightDir).normalize();
        f64 vDotH = std::max(viewDir.dot(halfDir), 0.0);
        Color fresnel = fresnel_schlick(vDotH, material.f0);
        Color diffuse = material.albedo / PI * (Color { 1.0, 1.0, 1.0 } - fresnel)
            * (1.0 - material.metalness) * (1.0 - material.transparency);
        f64 nDotH = std::max(normal.dot(halfDir), 0.0);
        f64 denom = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
        f64 distribution = alpha2 / (PI * denom * denom);
        f64 nDotL = std::max(normal.dot(lightDir), 0.0);
        f64 geometryL = nDotL / (nDotL * (1.0 - k) + k);
        f64 geometry = geometryV * geometryL;
        Color specular = fresnel
            * (distribution * geometry / std::max(4.0 * nDotV * nDotL, EPS));
        return (diffuse + specular) * nDotL;
    };
    Color local = { 0.0, 0.0, 0.0 };
    for (const PointLight& light : scene.pointLights) {
        Vec3<f64> lightPos = light.position;
        if (light.radius > 0.0) {
            Vec3<f64> approxDir = (lightPos - point).normalize();
            auto [tangent, bitangent] = orthonormal_basis(approxDir);
            Vec2<f64> sample = sample_disk(rng) * light.radius;
            lightPos += tangent * sample.x + bitangent * sample.y;
        }
        Vec3<f64> lightDir = lightPos - point;
        f64 distance = lightDir.norm();
        lightDir = lightDir.normalize();
        Color shadow = shadow_attenuation(lightDir, distance);
        if (shadow.max() > 0.0) {
            f64 attenuation = 1.0
                / (light.kc + light.kl * distance
                   + light.kq * distance * distance);
            local += lighting(lightDir) * light.color * light.intensity
                * attenuation * shadow;
        }
    }
    for (const SpotLight& light : scene.spotLights) {
        Vec3<f64> lightPos = light.position;
        if (light.radius > 0.0) {
            Vec3<f64> approxDir = (lightPos - point).normalize();
            auto [tangent, bitangent] = orthonormal_basis(approxDir);
            Vec2<f64> sample = sample_disk(rng) * light.radius;
            lightPos += tangent * sample.x + bitangent * sample.y;
        }
        Vec3<f64> lightDir = lightPos - point;
        f64 distance = lightDir.norm();
        lightDir = lightDir.normalize();
        Color shadow = shadow_attenuation(lightDir, distance);
        if (shadow.max() > 0.0) {
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
                * attenuation * shadow;
        }
    }
    for (const DirectionalLight& light : scene.directionalLights) {
        Vec3<f64> lightDir = (-light.direction).normalize();
        if (light.radius > 0.0) {
            auto [tangent, bitangent] = orthonormal_basis(lightDir);
            Vec2<f64> sample = sample_disk(rng) * light.radius;
            lightDir = (lightDir + tangent * sample.x + bitangent * sample.y)
                .normalize();
        }
        Color shadow = shadow_attenuation(lightDir, INF);
        if (shadow.max() > 0.0) {
            local += lighting(lightDir) * light.color * light.intensity
                * shadow;
        }
    }
    return local;
}

/**
 * @brief Computes the Beer-Lambert attenuation of light traveling through an
 * absorptive medium.
 *
 * @param[in] absorption The absorption coefficients of the medium.
 * @param[in] distance   The distance traveled through the medium.
 * @return The attenuation applied to light after traveling that distance.
 */
static Color beer_lambert(const Color& absorption, f64 distance) noexcept {
    return {
        .r = std::exp(-absorption.r * distance),
        .g = std::exp(-absorption.g * distance),
        .b = std::exp(-absorption.b * distance)
    };
}

/**
 * @brief Traces a ray through a scene and returns the surface color of the
 * closest object it intersects with.
 *
 * @param[in]      scene The scene containing objects and lights.
 * @param[in]      ray   The ray to trace.
 * @param[in, out] rng   The random engine to draw from.
 * @param[in]      depth The maximum recursion depth.
 * @return The surface color of the closest object the ray intersects with, or
 * `BACKGROUND_COLOR` if no intersection is found.
 */
static constexpr Color trace_ray(
    const Scene& scene,
    const Ray& ray,
    std::mt19937& rng,
    IorStack iorStack,
    isize depth = MAX_TRACE_DEPTH
) noexcept {
    assert(depth >= 0);
    Color result = BACKGROUND_COLOR;
    std::optional<Hit> hit = closest_intersection(scene.objects, ray);
    if (!hit) {
        return result;
    }
    const auto& [t, point, normal, material] = *hit;
    Vec3<f64> viewDir = (-ray.direction).normalize();
    Color local = local_color(point, normal, viewDir, material, scene, rng);
    if (depth == 0) {
        result = local;
        return result;
    }
    if (material.is_transparent()) {
        bool isEntering = ray.direction.dot(normal) < 0.0;
        Vec3<f64> n = isEntering ? normal : -normal;
        f64 previousIor = isEntering ? iorStack.current() : iorStack.previous();
        f64 eta = isEntering
            ? previousIor / material.ior
            : material.ior / previousIor;
        Color fresnel = fresnel_schlick(
            std::max(-ray.direction.dot(n), 0.0),
            material.f0
        );
        std::optional<Vec3<f64>> refractDir = ray.direction.refract(n, eta);
        std::uniform_real_distribution<f64> dist(0.0, 1.0);
        bool isReflected = !refractDir || (dist(rng) < fresnel.max());
        Vec3<f64> nextDir = isReflected ? viewDir.reflect(normal) : *refractDir;
        if (material.roughness > 0.0) {
            auto [tangent, bitangent] = orthonormal_basis(nextDir);
            f64 alpha = std::max(material.roughness * material.roughness, EPS);
            Vec2<f64> sample = sample_disk(rng) * alpha;
            nextDir = (nextDir + tangent * sample.x + bitangent * sample.y)
                .normalize();
        }
        if (nextDir.dot(n) * (isReflected ? 1.0 : -1.0) <= 0.0) {
            result = local;
            return result;
        }
        if (!isReflected) {
            if (isEntering) {
                iorStack.push(material.ior);
            }
            else {
                iorStack.pop();
            }
        }
        Color traced = trace_ray(
            scene,
            {
                .origin = offset_ray_origin(point, isReflected ? n : -n),
                .direction = nextDir,
                .tMin = 0.0,
                .tMax = INF
            },
            rng,
            iorStack,
            depth - 1
        );
        if (!isReflected && !isEntering) {
            traced *= beer_lambert(material.absorption, t);
        }
        result = Color::lerp(local, traced, material.transparency);
        return result;
    }
    f64 nDotV = std::max(normal.dot(viewDir), 0.0);
    f64 r = 1.0 - material.roughness;
    Color reflectance = fresnel_schlick(nDotV, material.f0) * r * r;
    if (reflectance.max() < EPS) {
        result = local;
        return result;
    }
    Vec3<f64> reflectDir = viewDir.reflect(normal);
    if (material.roughness > 0.0) {
        auto [tangent, bitangent] = orthonormal_basis(reflectDir);
        f64 alpha = std::max(material.roughness * material.roughness, EPS);
        Vec2<f64> sample = sample_disk(rng) * alpha;
        reflectDir = (reflectDir + tangent * sample.x + bitangent * sample.y)
            .normalize();
    }
    if (reflectDir.dot(normal) <= 0.0) {
        result = local;
        return result;
    }
    Color reflected = trace_ray(
        scene,
        {
            .origin = offset_ray_origin(point, normal),
            .direction = reflectDir,
            .tMin = 0.0,
            .tMax = INF
        },
        rng,
        iorStack,
        depth - 1
    );
    result = Color::lerp(local, reflected, reflectance);
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
    std::atomic<isize> nextY = 0;
    auto render_row = [&]() -> void {
        std::random_device device;
        std::mt19937 rng(device());
        isize y;
        while (
            (y = nextY.fetch_add(1, std::memory_order::relaxed))
                < canvas.height()
        ) {
            for (isize x = 0; x < canvas.width(); x++) {
                Color sum = { };
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
                        IorStack iorStack;
                        iorStack.push(1.0);
                        sum += trace_ray(scene, ray, rng, iorStack);
                    }
                }
                Color avg = sum / static_cast<f64>(samples * samples);
                Color color = gamma_correct(avg).clamp(0.0, 1.0);
                Pixel pixel = {
                    .r = static_cast<u8>(color.r * 255.0 + 0.5),
                    .g = static_cast<u8>(color.g * 255.0 + 0.5),
                    .b = static_cast<u8>(color.b * 255.0 + 0.5)
                };
                canvas.put_pixel(x, y, pixel);
            }
        }
    };
    std::vector<std::jthread> threads;
    threads.reserve(MAX_THREADS);
    for (usize i = 0; i < MAX_THREADS; i++) {
        threads.emplace_back(render_row);
    }
}

}