#include <cstdlib>
#include <vector>
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/util.hpp"

/** @brief The number of spheres in each row or column. */
static constexpr cgfs::isize GRID_SIZE = 5;

/** @brief The spacing between spheres. */
static constexpr cgfs::f64 GRID_SPACING = 1.25;

/** @brief The radius of each sphere. */
static constexpr cgfs::f64 SPHERE_RADIUS = 0.5;

/** @brief The size of the floor. */
static constexpr cgfs::isize FLOOR_SIZE = 100;

int main() {
    cgfs::Canvas canvas = cgfs::Canvas::empty(1920, 1080);
    std::vector<cgfs::Object> objects = {
        cgfs::Mesh {
            .vertices = {
                { -FLOOR_SIZE, -SPHERE_RADIUS, -FLOOR_SIZE },
                { -FLOOR_SIZE, -SPHERE_RADIUS, FLOOR_SIZE },
                { FLOOR_SIZE, -SPHERE_RADIUS, FLOOR_SIZE },
                { FLOOR_SIZE, -SPHERE_RADIUS, -FLOOR_SIZE }
            },
            .indices = {
                0, 1, 2,
                0, 2, 3
            },
            .bounds = std::nullopt,
            .material = cgfs::Material::opaque({ 0.6, 0.6, 0.6 }, 0.0, 0.6)
        }
    };
    for (cgfs::isize row = 0; row < GRID_SIZE; row++) {
        cgfs::f64 metalness = 1.0 - static_cast<cgfs::f64>(row)
            / static_cast<cgfs::f64>(GRID_SIZE - 1);
        for (cgfs::isize col = 0; col < GRID_SIZE; col++) {
            cgfs::f64 roughness = static_cast<cgfs::f64>(col)
                / static_cast<cgfs::f64>(GRID_SIZE - 1);
            objects.push_back(
                cgfs::Sphere {
                    .center = {
                        (static_cast<cgfs::f64>(col) - (GRID_SIZE - 1) / 2.0)
                            * GRID_SPACING,
                        0.0,
                        -6.0 - (static_cast<cgfs::f64>(row) - (GRID_SIZE - 1)
                            / 2.0) * GRID_SPACING
                    },
                    .radius = SPHERE_RADIUS,
                    .material = cgfs::Material::opaque(
                        { 0.9, 0.65, 0.2 },
                        metalness,
                        roughness
                    )
                }
            );
        }
    }
    cgfs::Scene scene = {
        .objects = objects,
        .pointLights = {
            {
                .color = { 1.0, 1.0, 1.0 },
                .intensity = 2.0,
                .position = { 3.0, 5.0, -3.0 },
                .radius = 1.0,
                .kc = 1.0,
                .kl = 0.09,
                .kq = 0.032
            }
        },
        .spotLights = { },
        .directionalLights = {
            {
                .color = { 0.6, 0.7, 1.0 },
                .intensity = 0.5,
                .direction = { -0.3, -1.0, -0.3 },
                .radius = 0.5
            }
        },
        .viewport = cgfs::Viewport::create(
            cgfs::radians(45.0),
            static_cast<cgfs::f64>(canvas.width())
                / static_cast<cgfs::f64>(canvas.height())
        ),
        .camera = cgfs::Camera::look_at({ 0.0, 4.0, 0.0 }, { 0.0, -0.5, -6.0 })
    };
    cgfs::raytrace(scene, canvas, 16);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}