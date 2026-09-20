#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>
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
    Vec3<f32> origin;

    /** @brief The direction of the ray. */
    Vec3<f32> direction;

    /** @brief The minimum distance to consider for intersections. */
    f32 tMin;

    /** @brief The maximum distance to consider for intersections. */
    f32 tMax;

};

/** @brief Represents an intersection of a ray with an object. */
struct Hit {

    /** @brief The distance along the ray to the intersection point. */
    f32 t;

    /** @brief The intersection point. */
    Vec3<f32> point;

    /** @brief The surface normal at the intersection point. */
    Vec3<f32> normal;

    /** @brief The material at the intersection point. */
    Material material;

};

/** @brief The minimum number of recursive bounces. */
static constexpr isize MIN_BOUNCES = 3;

/** @brief The maximum number of recursive bounces. */
static constexpr isize MAX_BOUNCES = 32;

/** @brief The minimum contribution for a ray to survive. */
static constexpr f32 MIN_SURVIVAL = 0.05F;

/**
 * @brief Represents a simple stack for tracking refractive indices of
 * transparent media.
 */
class IorStack final {
private:

    /** @brief The stack of refractive indices. */
    std::array<f32, MAX_BOUNCES + 1> _iors = { };

    /** @brief The current size of the stack. */
    isize _size = 0;

public:

    /**
     * @brief Returns the refractive index of the current medium, or `1.0F` if
     * this stack is empty.
     *
     * @return The refractive index of the current medium.
     */
    constexpr f32 current() const noexcept {
        return (_size > 0) ? _iors[_size - 1] : 1.0F;
    }

    /**
     * @brief Returns the refractive index of the previous medium, or `1.0F` if
     * this stack has fewer than two elements.
     *
     * @return The refractive index of the previous medium.
     */
    constexpr f32 previous() const noexcept {
        return (_size > 1) ? _iors[_size - 2] : 1.0F;
    }

    /**
     * @brief Enters a medium by pushing its refractive index onto this stack.
     *
     * @warning The behavior is undefined if this stack is full, or if `ior` is
     * not greater than zero.
     *
     * @param[in] ior The refractive index of the entered medium.
     */
    constexpr void push(f32 ior) noexcept {
        assert(_size < std::ssize(_iors));
        assert(ior > 0.0F);
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

/** @brief Represents a pseudorandom number generator. */
class Rng final {
private:

    /** @brief The internal state. */
    std::array<u32, 4> _state;

    /**
     * @brief Generates an initial state based on a specified seed.
     *
     * @param[in] seed The seed for the initialization.
     * @return The initial state for the pseudorandom number generator.
     */
    static constexpr std::array<u32, 4> make_state(u64 seed) noexcept {
        auto splitmix64 = [](u64& state) {
            state += 0x9E3779B97F4A7C15ULL;
            u64 z = state;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        };
        u64 a = splitmix64(seed);
        u64 b = splitmix64(seed);
        return {
            static_cast<u32>(a),
            static_cast<u32>(a >> 32),
            static_cast<u32>(b),
            static_cast<u32>(b >> 32)
        };
    }

public:

    /**
     * @brief Initializes a pseudorandom number generator with a specified seed.
     *
     * @param[in] seed The seed for the initialization.
     */
    constexpr explicit Rng(u64 seed) noexcept : _state(make_state(seed)) { }

    /**
     * @brief Generates a pseudorandom `u32`.
     *
     * @return A pseudorandom `u32`.
     */
    constexpr u32 next_u32() noexcept {
        u32 result = std::rotl(_state[1] * 5, 7) * 9;
        u32 t = _state[1] << 9;
        _state[2] ^= _state[0];
        _state[3] ^= _state[1];
        _state[1] ^= _state[2];
        _state[0] ^= _state[3];
        _state[2] ^= t;
        _state[3] = std::rotl(_state[3], 11);
        return result;
    }

    /**
     * @brief Generates a pseudorandom `f32` in the range `[0, 1)`.
     *
     * @return A pseudorandom `f32` in the range `[0, 1)`.
     */
    constexpr f32 next_f32() noexcept {
        return static_cast<f32>(next_u32() >> 8) * (1.0F / 16777216.0F);
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
static constexpr f32 PI = std::numbers::pi_v<f32>;

/** @brief Represents infinity for floating-point calculations. */
static constexpr f32 INF = std::numeric_limits<f32>::infinity();

/** @brief The epsilon for floating-point calculations. */
static constexpr f32 EPS = 1.0E-6F;

/** @brief The background color used when no objects are intersected. */
static constexpr Color BACKGROUND_COLOR = { 0.0F, 0.0F, 0.0F };

/**
 * @brief The maximum depth for determining the shadow attenuation through
 * transparent objects.
 */
static constexpr isize MAX_TRANSPARENCY_DEPTH = 8;

/** @brief The factor used for gamma correction. */
static constexpr f32 GAMMA = 2.2F;

/** @brief The maximum number of threads to use for rendering. */
static const usize MAX_THREADS = std::max<usize>(
    std::thread::hardware_concurrency(),
    1
);

/**
 * @brief Tries to intersect a ray with a sphere.
 *
 * @param[in]  sphere     The sphere to test for intersection.
 * @param[in]  ray        The ray to test for intersection.
 * @param[out] closestHit The hit information if an intersection is found.
 * @return `true` if an intersection is found, otherwise `false`.
 */
static constexpr bool intersect_sphere(
    const Sphere& sphere,
    const Ray& ray,
    Hit& closestHit
) noexcept {
    Vec3<f32> co = ray.origin - sphere.center;
    f32 a = ray.direction.dot(ray.direction);
    f32 b = co.dot(ray.direction);
    f32 c = co.dot(co) - sphere.radius * sphere.radius;
    f32 d = b * b - a * c;
    if (d < 0.0F) {
        return false;
    }
    f32 q = -(b + std::copysign(std::sqrt(d), b));
    f32 t0 = q / a;
    f32 t1 = c / q;
    if (t0 > t1) {
        std::swap(t0, t1);
    }
    if ((t0 >= ray.tMin) && (t0 <= ray.tMax)) {
        Vec3<f32> point = ray.origin + t0 * ray.direction;
        Vec3<f32> normal = (point - sphere.center).normalize();
        closestHit = { t0, point, normal, sphere.material };
        return true;
    }
    if ((t1 >= ray.tMin) && (t1 <= ray.tMax)) {
        Vec3<f32> point = ray.origin + t1 * ray.direction;
        Vec3<f32> normal = (point - sphere.center).normalize();
        closestHit = { t1, point, normal, sphere.material };
        return true;
    }
    return false;
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
    f32 tMin = ray.tMin;
    f32 tMax = ray.tMax;
    auto slab = [&](f32 min, f32 max, f32 origin, f32 dir) {
        f32 invDir = 1.0F / dir;
        f32 t0 = (min - origin) * invDir;
        f32 t1 = (max - origin) * invDir;
        if (invDir < 0.0F) {
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
 * @brief Tries to intersect a ray with a triangle.
 *
 * @param[in]  triangle The triangle to test for intersection.
 * @param[in]  ray      The ray to test for intersection.
 * @param[out] t        The intersection distance if an intersection is found.
 * @return `true` if an intersection is found, otherwise `false`.
 */
static constexpr bool intersect_triangle(
    const Triangle& triangle,
    const Ray& ray,
    f32& t
) noexcept {
    Vec3<f32> e0 = triangle.v1 - triangle.v0;
    Vec3<f32> e1 = triangle.v2 - triangle.v0;
    Vec3<f32> a = ray.direction.cross(e1);
    f32 det = e0.dot(a);
    if (std::abs(det) < EPS) {
        return false;
    }
    f32 invDet = 1.0F / det;
    Vec3<f32> b = ray.origin - triangle.v0;
    f32 u = b.dot(a) * invDet;
    if ((u < 0.0F) || (u > 1.0F)) {
        return false;
    }
    Vec3<f32> c = b.cross(e0);
    f32 v = ray.direction.dot(c) * invDet;
    if ((v < 0.0F) || (u + v > 1.0F)) {
        return false;
    }
    t = e1.dot(c) * invDet;
    return (t >= ray.tMin) && (t <= ray.tMax);
}

/**
 * @brief Tries to intersect a ray with a mesh.
 *
 * @param[in]  mesh       The mesh to test for intersection.
 * @param[in]  ray        The ray to test for intersection.
 * @param[out] closestHit The hit information if an intersection is found.
 * @return `true` if an intersection is found, otherwise `false`.
 */
static constexpr bool intersect_mesh(
    const Mesh& mesh,
    const Ray& ray,
    Hit& closestHit
) noexcept {
    if (mesh.bounds && !intersect_aabb(*mesh.bounds, ray)) {
        return false;
    }
    bool foundHit = false;
    for (const Triangle& triangle : mesh.triangles()) {
        f32 t;
        if (
            intersect_triangle(triangle, ray, t)
                && (!foundHit || (t < closestHit.t))
        ) {
            closestHit = {
                t,
                ray.origin + ray.direction * t,
                triangle.normal(),
                mesh.material
            };
            foundHit = true;
        }
    }
    return foundHit;
}

/**
 * @brief Tries to intersect a ray with a collection of objects.
 *
 * @param[in]  objects    A collection of objects to test for intersections.
 * @param[in]  ray        The ray to test for intersections.
 * @param[out] closestHit The hit information if an intersection is found.
 * @return `true` if an intersection is found, otherwise `false`.
 */
static constexpr bool intersect(
    std::span<const Object> objects,
    const Ray& ray,
    Hit& closestHit
) noexcept {
    bool foundHit = false;
    for (const Object& object : objects) {
        Ray updatedRay = {
            ray.origin,
            ray.direction,
            ray.tMin,
            foundHit ? closestHit.t : ray.tMax
        };
        Hit hit;
        bool isHit = std::visit(
            Overloaded {
                [&](const Sphere& sphere) {
                    return intersect_sphere(sphere, updatedRay, hit);
                },
                [&](const Mesh& mesh) {
                    return intersect_mesh(mesh, updatedRay, hit);
                }
            },
            object
        );
        if (isHit && (!foundHit || (hit.t < closestHit.t))) {
            closestHit = hit;
            foundHit = true;
        }
    }
    return foundHit;
}

/**
 * @brief Offsets a ray origin to avoid self-intersections.
 *
 * @param[in] point  The original point of intersection.
 * @param[in] normal The normal at the intersection point.
 * @return The offset ray origin.
 */
static constexpr Vec3<f32> offset_ray_origin(
    const Vec3<f32>& point,
    const Vec3<f32>& normal
) noexcept {
    constexpr f32 ORIGIN = 1.0F / 32.0F;
    constexpr f32 FLOAT_SCALE = 1.0F / 65536.0F;
    constexpr f32 INT_SCALE = 256.0F;
    auto offset = [](f32 p, f32 n) {
        i32 i = static_cast<i32>(INT_SCALE * n);
        i32 bits = std::bit_cast<i32>(p);
        f32 shifted = std::bit_cast<f32>(bits + ((p < 0.0F) ? -i : i));
        return (std::abs(p) < ORIGIN) ? p + FLOAT_SCALE * n : shifted;
    };
    return {
        offset(point.x, normal.x),
        offset(point.y, normal.y),
        offset(point.z, normal.z)
    };
}

/**
 * @brief Returns an orthonormal basis around a normal.
 *
 * @param[in] normal The normal around which to compute the basis (normalized).
 * @return A pair containing the tangent and bitangent forming an orthonormal
 * basis with the normal.
 */
static constexpr std::pair<Vec3<f32>, Vec3<f32>> orthonormal_basis(
    const Vec3<f32>& normal
) noexcept {
    f32 sign = std::copysign(1.0F, normal.z);
    f32 a = -1.0F / (sign + normal.z);
    f32 b = normal.x * normal.y * a;
    return {
        { 1.0F + sign * normal.x * normal.x * a, sign * b, -sign * normal.x },
        { b, sign + normal.y * normal.y * a, -normal.y }
    };
}

/**
 * @brief Uniformly samples a point on a unit disk.
 *
 * @param[in, out] rng The pseudorandom number generator to draw from.
 * @return A point on the unit disk.
 */
static constexpr Vec2<f32> sample_disk(Rng& rng) noexcept {
    f32 r = std::sqrt(rng.next_f32());
    f32 theta = rng.next_f32() * 2.0F * PI;
    return { r * std::cos(theta), r * std::sin(theta) };
}

/**
 * @brief Perturbs a direction vector based on the roughness of a surface.
 *
 * @param[in]      direction The original direction (normalized).
 * @param[in]      roughness The roughness of the surface.
 * @param[in, out] rng       The pseudorandom number generator to draw from.
 * @return The perturbed direction vector.
 */
static constexpr Vec3<f32> perturb_direction(
    const Vec3<f32>& direction,
    f32 roughness,
    Rng& rng
) noexcept {
    auto [tangent, bitangent] = orthonormal_basis(direction);
    f32 alpha = std::max(roughness * roughness, EPS);
    Vec2<f32> sample = sample_disk(rng) * alpha;
    return (direction + tangent * sample.x + bitangent * sample.y).normalize();
}

/**
 * @brief Uniformly samples a point on a hemisphere around a normal.
 *
 * @param[in]      normal The normal around which to sample (normalized).
 * @param[in, out] rng    The pseudorandom number generator to draw from.
 * @return A point on the hemisphere around the normal.
 */
static constexpr Vec3<f32> sample_hemisphere(
    const Vec3<f32>& normal,
    Rng& rng
) noexcept {
    Vec2<f32> disk = sample_disk(rng);
    f32 z = std::sqrt(std::max(0.0F, 1.0F - disk.x * disk.x - disk.y * disk.y));
    auto [tangent, bitangent] = orthonormal_basis(normal);
    return (tangent * disk.x + bitangent * disk.y + normal * z).normalize();
}

/**
 * @brief Returns the Fresnel reflectance using the Schlick approximation.
 *
 * @param[in] cosTheta The cosine of the angle between the view/light direction
 *                     and the half-vector (or normal).
 * @param[in] f0       The reflectance at normal incidence.
 * @return The Fresnel reflectance.
 */
static constexpr Color fresnel_schlick(f32 cosTheta, const Color& f0) noexcept {
    f32 x = 1.0F - std::clamp(cosTheta, 0.0F, 1.0F);
    f32 t = x * x * x * x * x;
    return Color::lerp(f0, { 1.0F, 1.0F, 1.0F }, t);
}

/**
 * @brief Returns the local color at a point on a surface, accounting for the
 * material properties, lights, and other objects in the scene.
 *
 * @param[in]      point    The point to compute the local color for.
 * @param[in]      normal   The surface normal at that point (normalized).
 * @param[in]      viewDir  The direction from the point towards the camera
 *                          (normalized).
 * @param[in]      material The material of the surface at the point.
 * @param[in]      scene    The scene containing objects and lights.
 * @param[in, out] rng      The pseudorandom number generator to draw from.
 * @return The local color at the point.
 */
static constexpr Color local_color(
    const Vec3<f32>& point,
    const Vec3<f32>& normal,
    const Vec3<f32>& viewDir,
    const Material& material,
    const Scene& scene,
    Rng& rng
) noexcept {
    auto jitter_position = [&](const Vec3<f32>& pos, f32 radius) {
        if (radius <= 0.0F) {
            return pos;
        }
        Vec3<f32> approxDir = (pos - point).normalize();
        auto [tangent, bitangent] = orthonormal_basis(approxDir);
        Vec2<f32> sample = sample_disk(rng) * radius;
        return pos + tangent * sample.x + bitangent * sample.y;
    };
    auto jitter_direction = [&](const Vec3<f32>& dir, f32 radius) {
        if (radius <= 0.0F) {
            return dir;
        }
        auto [tangent, bitangent] = orthonormal_basis(dir);
        Vec2<f32> sample = sample_disk(rng) * radius;
        return (dir + tangent * sample.x + bitangent * sample.y).normalize();
    };
    f32 alpha = std::max(material.roughness * material.roughness, EPS);
    f32 alpha2 = alpha * alpha;
    f32 nDotV = std::max(normal.dot(viewDir), 0.0F);
    f32 k = (material.roughness + 1.0F) * (material.roughness + 1.0F) / 8.0F;
    f32 geometryV = nDotV / (nDotV * (1.0F - k) + k);
    auto shading = [&](const Vec3<f32>& lightDir) {
        Vec3<f32> halfDir = (viewDir + lightDir).normalize();
        f32 vDotH = std::max(viewDir.dot(halfDir), 0.0F);
        Color fresnel = fresnel_schlick(vDotH, material.f0);
        Color diffuse = material.albedo / PI
            * (Color { 1.0F, 1.0F, 1.0F } - fresnel)
            * (1.0F - material.metalness) * (1.0F - material.transparency);
        f32 nDotH = std::max(normal.dot(halfDir), 0.0F);
        f32 denom = nDotH * nDotH * (alpha2 - 1.0F) + 1.0F;
        f32 distribution = alpha2 / (PI * denom * denom);
        f32 nDotL = std::max(normal.dot(lightDir), 0.0F);
        f32 geometryL = nDotL / (nDotL * (1.0F - k) + k);
        f32 geometry = geometryV * geometryL;
        Color specular = fresnel
            * (distribution * geometry / std::max(4.0F * nDotV * nDotL, EPS));
        return (diffuse + specular) * nDotL;
    };
    auto distance_attenuation = [](const auto& light, f32 distance) {
        return 1.0F
            / (light.kc + light.kl * distance + light.kq * distance * distance);
    };
    auto shadow_attenuation = [](
        std::span<const Object> objects,
        Ray shadowRay
    ) {
        Color attenuation = { 1.0F, 1.0F, 1.0F };
        for (isize i = 0; i < MAX_TRANSPARENCY_DEPTH; i++) {
            Hit hit;
            if (!intersect(objects, shadowRay, hit)) {
                break;
            }
            const Material& mat = hit.material;
            if (mat.is_opaque()) {
                attenuation = { 0.0F, 0.0F, 0.0F };
                break;
            }
            attenuation *= mat.albedo * mat.transparency
                + Color { 1.0F, 1.0F, 1.0F } * (1.0F - mat.transparency);
            if (attenuation.max() < EPS) {
                attenuation = { 0.0F, 0.0F, 0.0F };
                break;
            }
            shadowRay.origin = offset_ray_origin(hit.point, hit.normal);
            shadowRay.tMax -= hit.t;
        }
        return attenuation;
    };
    Color local = { 0.0F, 0.0F, 0.0F };
    for (const PointLight& light : scene.pointLights) {
        Vec3<f32> lightPos = jitter_position(light.position, light.radius);
        Vec3<f32> lightDir = lightPos - point;
        f32 distance = lightDir.norm();
        lightDir /= distance;
        Ray shadowRay = {
            offset_ray_origin(point, lightDir),
            lightDir,
            0.0F,
            distance
        };
        local += shading(lightDir) * light.color * light.intensity
            * distance_attenuation(light, distance)
            * shadow_attenuation(scene.objects, shadowRay);
    }
    for (const SpotLight& light : scene.spotLights) {
        Vec3<f32> lightPos = jitter_position(light.position, light.radius);
        Vec3<f32> lightDir = lightPos - point;
        f32 distance = lightDir.norm();
        lightDir /= distance;
        Ray shadowRay = {
            offset_ray_origin(point, lightDir),
            lightDir,
            0.0F,
            distance
        };
        f32 cosTheta = (-lightDir).dot(light.direction);
        if (cosTheta <= light.outerCutoff) {
            continue;
        }
        f32 spot = std::clamp(
            (cosTheta - light.outerCutoff)
                / (light.innerCutoff - light.outerCutoff),
            0.0F,
            1.0F
        );
        local += shading(lightDir) * light.color * light.intensity
            * distance_attenuation(light, distance)
            * shadow_attenuation(scene.objects, shadowRay) * spot;
    }
    for (const DirectionalLight& light : scene.directionalLights) {
        Vec3<f32> lightDir = jitter_direction(
            (-light.direction).normalize(),
            light.radius
        );
        Ray shadowRay = {
            offset_ray_origin(point, lightDir),
            lightDir,
            0.0F,
            INF
        };
        local += shading(lightDir) * light.color * light.intensity
            * shadow_attenuation(scene.objects, shadowRay);
    }
    return local;
}

/**
 * @brief Decides whether a ray should keep bouncing.
 *
 * @param[in]      depth        The remaining recursion depth.
 * @param[in]      contribution The current contribution of the ray.
 * @param[in, out] rng          The pseudorandom number generator to draw from.
 * @return The compensation weight if the ray survives, or `std::nullopt` if it
 * should be terminated.
 */
static constexpr std::optional<f32> russian_roulette(
    isize depth,
    const Color& contribution,
    Rng& rng
) noexcept {
    isize bounces = MAX_BOUNCES - depth;
    if (bounces < MIN_BOUNCES) {
        return 1.0F;
    }
    f32 survival = std::clamp(contribution.max(), MIN_SURVIVAL, 1.0F);
    if (rng.next_f32() >= survival) {
        return std::nullopt;
    }
    return 1.0F / survival;
}

/**
 * @brief Returns the Beer-Lambert attenuation of light traveling through an
 * absorptive medium.
 *
 * @param[in] absorption The absorption coefficients of the medium.
 * @param[in] distance   The distance traveled through the medium.
 * @return The attenuation applied to light after traveling that distance.
 */
static Color beer_lambert(const Color& absorption, f32 distance) noexcept {
    return {
        std::exp(-absorption.r * distance),
        std::exp(-absorption.g * distance),
        std::exp(-absorption.b * distance)
    };
}

/**
 * @brief Traces a ray through a scene and returns the surface color of the
 * closest object it intersects with.
 *
 * @param[in]      scene        The scene containing objects and lights.
 * @param[in]      ray          The ray to trace.
 * @param[in, out] rng          The pseudorandom number generator to draw from.
 * @param[in, out] iorStack     The stack of indices of refraction for nested
 *                              transparent materials.
 * @param[in]      depth        The maximum recursion depth.
 * @param[in]      contribution The current contribution of the ray.
 * @return The surface color of the closest object the ray intersects with, or
 * `BACKGROUND_COLOR` if no intersection is found.
 */
static constexpr Color trace_ray(
    const Scene& scene,
    const Ray& ray,
    Rng& rng,
    IorStack& iorStack,
    isize depth = MAX_BOUNCES,
    Color contribution = { 1.0F, 1.0F, 1.0F }
) noexcept {
    assert(depth >= 0);
    Hit hit;
    if (!intersect(scene.objects, ray, hit)) {
        return BACKGROUND_COLOR;
    }
    const auto& [t, point, normal, material] = hit;
    Vec3<f32> viewDir = -ray.direction;
    Color local = local_color(point, normal, viewDir, material, scene, rng)
        + material.emission;
    if (depth == 0) {
        return local;
    }
    std::optional<f32> weight = russian_roulette(depth, contribution, rng);
    if (!weight) {
        return local;
    }
    if (material.is_transparent()) {
        bool isEntering = ray.direction.dot(normal) < 0.0F;
        Vec3<f32> n = isEntering ? normal : -normal;
        f32 previousIor = isEntering ? iorStack.current() : iorStack.previous();
        f32 eta = isEntering
            ? previousIor / material.ior
            : material.ior / previousIor;
        std::optional<Vec3<f32>> refractDir = ray.direction.refract(n, eta);
        Color fresnel = fresnel_schlick(
            std::max(viewDir.dot(n), 0.0F),
            material.f0
        );
        bool isReflected = !refractDir || (rng.next_f32() < fresnel.max());
        Vec3<f32> nextDir = isReflected ? viewDir.reflect(normal) : *refractDir;
        if (material.roughness > 0.0F) {
            nextDir = perturb_direction(nextDir, material.roughness, rng);
        }
        if (nextDir.dot(n) * (isReflected ? 1.0F : -1.0F) <= 0.0F) {
            return local;
        }
        if (!isReflected) {
            if (isEntering) {
                iorStack.push(material.ior);
            }
            else {
                iorStack.pop();
            }
        }
        Ray nextRay = {
            offset_ray_origin(point, isReflected ? n : -n),
            nextDir,
            0.0F,
            INF
        };
        Color traced = *weight * trace_ray(
            scene,
            nextRay,
            rng,
            iorStack,
            depth - 1,
            contribution * *weight
        );
        if (!isReflected) {
            if (isEntering) {
                iorStack.pop();
            }
            else {
                iorStack.push(material.ior);
                traced *= beer_lambert(material.absorption, t);
            }
        }
        return Color::lerp(local, traced, material.transparency);
    }
    else {
        f32 nDotV = std::max(normal.dot(viewDir), 0.0F);
        f32 r = 1.0F - material.roughness;
        Color specWeight = fresnel_schlick(nDotV, material.f0) * r * r;
        Color diffWeight = material.albedo * (1.0F - material.metalness);
        f32 totalWeight = specWeight.max() + diffWeight.max();
        if (totalWeight < EPS) {
            return local;
        }
        f32 pSpecular = std::clamp(
            specWeight.max() / totalWeight,
            0.05F,
            0.95F
        );
        Vec3<f32> nextDir;
        Color lobeWeight;
        if (rng.next_f32() < pSpecular) {
            nextDir = viewDir.reflect(normal);
            if (material.roughness > 0.0F) {
                nextDir = perturb_direction(nextDir, material.roughness, rng);
            }
            if (nextDir.dot(normal) <= 0.0F) {
                return local;
            }
            lobeWeight = specWeight / pSpecular;
        }
        else {
            nextDir = sample_hemisphere(normal, rng);
            lobeWeight = diffWeight / (1.0F - pSpecular);
        }
        Ray nextRay = { offset_ray_origin(point, normal), nextDir, 0.0F, INF };
        Color traced = trace_ray(
            scene,
            nextRay,
            rng,
            iorStack,
            depth - 1,
            contribution * *weight * lobeWeight
        );
        return local + traced * *weight * lobeWeight;
    }
}

/**
 * @brief Applies gamma correction to a color.
 *
 * @param[in] color The color to correct.
 * @return The gamma-corrected color.
 */
static constexpr Color gamma_correct(const Color& color) noexcept {
    return {
        std::pow(color.r, 1.0F / GAMMA),
        std::pow(color.g, 1.0F / GAMMA),
        std::pow(color.b, 1.0F / GAMMA)
    };
}

/**
 * @brief Renders a single row on a canvas using raytracing.
 *
 * @warning The behavior is undefined if `samples` is not greater than zero, or
 * if `y` is out of bounds.
 *
 * @param[in]  scene   The scene to render.
 * @param[out] canvas  The canvas to render the scene to.
 * @param[in]  samples The number of samples per pixel.
 * @param[in]  y       The index of the row to render.
 */
static constexpr void render_row(
    const Scene& scene,
    Canvas& canvas,
    isize samples,
    isize y
) noexcept {
    assert(samples > 0);
    assert((y >= 0) && (y < canvas.height()));
    Vec2<f32> canvasSize = {
        static_cast<f32>(canvas.width()),
        static_cast<f32>(canvas.height())
    };
    Vec2<f32> viewportScale = {
        scene.viewport.width / canvasSize.x,
        scene.viewport.height / canvasSize.y
    };
    std::random_device device;
    Rng rng(device());
    for (isize x = 0; x < canvas.width(); x++) {
        Color sum = { };
        for (isize sy = 0; sy < samples; sy++) {
            for (isize sx = 0; sx < samples; sx++) {
                Vec3<f32> viewportPos = {
                    (-canvasSize.x / 2.0F + static_cast<f32>(x)
                        + (static_cast<f32>(sx) + 0.5F)
                            / static_cast<f32>(samples))
                        * viewportScale.x,
                    (canvasSize.y / 2.0F - 1.0F - static_cast<f32>(y)
                        - (static_cast<f32>(sy) + 0.5F)
                            / static_cast<f32>(samples))
                        * viewportScale.y,
                    -scene.viewport.distance
                };
                Ray ray = {
                    scene.camera.position(),
                    (scene.camera.rotation() * viewportPos).normalize(),
                    scene.viewport.distance,
                    INF
                };
                IorStack iorStack;
                iorStack.push(1.0F);
                sum += trace_ray(scene, ray, rng, iorStack);
            }
        }
        Color avg = sum / static_cast<f32>(samples * samples);
        Color color = gamma_correct(avg).clamp(0.0F, 1.0F);
        Pixel pixel = {
            static_cast<u8>(color.r * 255.0F + 0.5F),
            static_cast<u8>(color.g * 255.0F + 0.5F),
            static_cast<u8>(color.b * 255.0F + 0.5F)
        };
        canvas.put_pixel(x, y, pixel);
    }
}

void raytrace(const Scene& scene, Canvas& canvas, isize samples) {
    assert(samples > 0);
    std::atomic<isize> nextY = 0;
    auto worker = [&]() {
        while (true) {
            isize y = nextY.fetch_add(1, std::memory_order::relaxed);
            if (y >= canvas.height()) {
                break;
            }
            render_row(scene, canvas, samples, y);
        }
    };
    std::vector<std::jthread> threads;
    threads.reserve(MAX_THREADS);
    for (usize i = 0; i < MAX_THREADS; i++) {
        threads.emplace_back(worker);
    }
}

}