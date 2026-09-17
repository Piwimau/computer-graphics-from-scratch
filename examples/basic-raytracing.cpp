#include <cstdlib>
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/util.hpp"

int main() {
    cgfs::Canvas canvas = cgfs::Canvas::empty(1920, 1080);
    cgfs::Scene scene = {
        .objects = {
            cgfs::Sphere {
                .center = { -2.0F, 0.0F, -4.0F },
                .radius = 1.0F,
                .material = cgfs::Material::opaque(
                    { 0.0F, 1.0F, 0.0F },
                    0.25F,
                    0.25F
                )
            },
            cgfs::Sphere {
                .center = { 0.0F, -1.0F, -3.0F },
                .radius = 1.0F,
                .material = cgfs::Material::opaque(
                    { 1.0F, 0.0F, 0.0F },
                    0.1F,
                    0.75F
                )
            },
            cgfs::Sphere {
                .center = { 2.0F, 0.0F, -4.0F },
                .radius = 1.0F,
                .material = cgfs::Material::opaque(
                    { 0.0F, 0.0F, 1.0F },
                    0.35F,
                    0.25F
                )
            },
            cgfs::Mesh {
                .vertices = {
                    { -50.0F, -1.0F, -50.0F },
                    { -50.0F, -1.0F, 50.0F },
                    { 50.0F, -1.0F, 50.0F },
                    { 50.0F, -1.0F, -50.0F }
                },
                .indices = {
                    0, 1, 2,
                    0, 2, 3
                },
                .bounds = std::nullopt,
                .material = cgfs::Material::opaque(
                    { 1.0F, 1.0F, 0.0F },
                    0.25F,
                    0.25F
                )
            }
        },
        .pointLights = {
            {
                .color = { 1.0F, 1.0F, 1.0F },
                .intensity = 1.0F,
                .position = { 2.0F, 1.0F, 0.0F },
                .radius = 1.0F,
                .kc = 1.0F,
                .kl = 0.09F,
                .kq = 0.032F
            }
        },
        .spotLights = { },
        .directionalLights = {
            {
                .color = { 1.0F, 1.0F, 1.0F },
                .intensity = 1.0F,
                .direction = { -4.0F, -1.0F, -1.0F },
                .radius = 1.0F
            }
        },
        .viewport = cgfs::Viewport::create(
            cgfs::radians(60.0F),
            static_cast<cgfs::f32>(canvas.width())
                / static_cast<cgfs::f32>(canvas.height())
        ),
        .camera = cgfs::Camera::look(
            { 0.0F, 0.0F, 0.0F },
            { 0.0F, 0.0F, -1.0F }
        )
    };
    cgfs::raytrace(scene, canvas, 16);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}