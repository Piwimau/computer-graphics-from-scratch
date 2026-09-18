#include <cstdlib>
#include <vector>
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/util.hpp"

/** @brief The number of spheres in each row or column. */
static constexpr cgfs::isize GRID_SIZE = 5;

/** @brief The spacing between spheres. */
static constexpr cgfs::f32 GRID_SPACING = 1.25F;

int main() {
    cgfs::Canvas canvas = cgfs::Canvas::empty(1920, 1080);
    std::vector<cgfs::Object> objects = {
        cgfs::Mesh {
            .vertices = {
                { -50.0F, -0.5F, -50.0F },
                { -50.0F, -0.5F, 50.0F },
                { 50.0F, -0.5F, 50.0F },
                { 50.0F, -0.5F, -50.0F }
            },
            .indices = {
                0, 1, 2,
                0, 2, 3
            },
            .bounds = std::nullopt,
            .material = cgfs::Material::opaque({ 0.6F, 0.6F, 0.6F }, 0.0F, 0.6F)
        }
    };
    for (cgfs::isize row = 0; row < GRID_SIZE; row++) {
        cgfs::f32 metalness = 1.0F - static_cast<cgfs::f32>(row)
            / static_cast<cgfs::f32>(GRID_SIZE - 1);
        for (cgfs::isize col = 0; col < GRID_SIZE; col++) {
            cgfs::f32 roughness = static_cast<cgfs::f32>(col)
                / static_cast<cgfs::f32>(GRID_SIZE - 1);
            objects.push_back(
                cgfs::Sphere {
                    .center = {
                        (static_cast<cgfs::f32>(col) - (GRID_SIZE - 1) / 2.0F)
                            * GRID_SPACING,
                        0.0F,
                        -6.0F - (static_cast<cgfs::f32>(row) - (GRID_SIZE - 1)
                            / 2.0F) * GRID_SPACING
                    },
                    .radius = 0.5F,
                    .material = cgfs::Material::opaque(
                        { 0.9F, 0.65F, 0.2F },
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
                .color = { 1.0F, 1.0F, 1.0F },
                .intensity = 2.0F,
                .position = { 3.0F, 5.0F, -3.0F },
                .radius = 1.0F,
                .kc = 1.0F,
                .kl = 0.09F,
                .kq = 0.032F
            }
        },
        .spotLights = { },
        .directionalLights = {
            {
                .color = { 0.6F, 0.7F, 1.0F },
                .intensity = 0.5F,
                .direction = { -0.3F, -1.0F, -0.3F },
                .radius = 0.5F
            }
        },
        .viewport = cgfs::Viewport::create(
            cgfs::radians(45.0F),
            static_cast<cgfs::f32>(canvas.width())
                / static_cast<cgfs::f32>(canvas.height())
        ),
        .camera = cgfs::Camera::look_at(
            { 0.0F, 4.0F, 0.0F },
            { 0.0F, -0.5F, -6.0F }
        )
    };
    cgfs::raytrace(scene, canvas, 32);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}