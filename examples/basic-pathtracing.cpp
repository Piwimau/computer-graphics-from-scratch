#include <cstdlib>
#include <format>
#include <print>
#include "cgfs/cgfs.hpp"
#include "sdl-context.hpp"
#include "sdl-delta-timer.hpp"
#include "sdl-fps-counter.hpp"
#include "sdl-window.hpp"

using namespace cgfs;

/** @brief The width of the window. */
static constexpr isize WIDTH = 1280;

/** @brief The height of the window. */
static constexpr isize HEIGHT = 720;

/** @brief The exposure multiplier for the film. */
static constexpr f32 EXPOSURE = 0.75;

/** @brief The mouse sensitivity for rotating the camera. */
static constexpr f32 ROTATION_SENSIVITY = 0.002F;

/** @brief The mouse sensitivity for zooming the camera. */
static constexpr f32 ZOOM_SENSIVITY = cgfs::radians(2.0F);

/** @brief The movement speed of the camera. */
static constexpr f32 SPEED = 5.0F;

/** @brief The movement speed of the camera when sprinting. */
static constexpr f32 SPRINT_SPEED = 15.0F;

static constexpr Scene make_scene() {
    return {
        .spheres = {
            {
                .center = { -2.0F, 0.0F, -4.0F },
                .radius = 1.0F,
                .material = Material::opaque()
                    .albedo({ 1.0F, 1.0F, 1.0F })
                    .metalness(0.95F)
                    .roughness(0.05F)
                    .build()
            },
            {
                .center = { 0.0F, -1.0F, -3.0F },
                .radius = 1.0F,
                .material = Material::opaque()
                    .albedo({ 1.0F, 0.0F, 0.0F })
                    .metalness(0.25F)
                    .roughness(0.25F)
                    .build()
            },
            {
                .center = { 2.0F, 0.0F, -4.0F },
                .radius = 1.0F,
                .material = Material::opaque()
                    .albedo({ 0.0F, 1.0F, 0.0F })
                    .metalness(0.5F)
                    .roughness(0.25F)
                    .build()
            },
            {
                .center = { 0.0F, -251.0F, 0.0F },
                .radius = 250.0F,
                .material = Material::opaque()
                    .albedo({ 1.0F, 1.0F, 0.0F })
                    .build()
            }
        },
        .lights = {
            Light::point().position({ 2.0F, 1.0F, 0.0F }).build(),
            Light::directional().direction({ -4.0F, -1.0F, -1.0F }).build()
        },
        .camera = Camera::builder().build()
    };
}

/**
 * @brief Handles pending SDL events and inputs.
 *
 * @param[in, out] deltaTimer The timer used to measure the delta time.
 * @param[in, out] camera     The camera through which the scene is viewed.
 * @param[in, out] film       The film to render the scene to.
 * @return `true` if the application should continue, otherwise `false`.
 */
static bool handle_events_and_inputs(
    SdlDeltaTimer& deltaTimer,
    Camera& camera,
    Film& film
) noexcept {
    bool clearFilm = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    return false;
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                camera.adjust_pitch(-event.motion.yrel * ROTATION_SENSIVITY);
                camera.adjust_yaw(-event.motion.xrel * ROTATION_SENSIVITY);
                clearFilm = true;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                camera.adjust_fov(-event.wheel.y * ZOOM_SENSIVITY);
                clearFilm = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_MIDDLE) {
                    camera.set_fov(Camera::DEFAULT_FOV);
                    clearFilm = true;
                }
                break;
            default:
                break;
        }
    }
    const bool* keys = SDL_GetKeyboardState(nullptr);
    auto axis = [&](SDL_Scancode pos, SDL_Scancode neg) {
        return (keys[pos] ? 1.0F : 0.0F) - (keys[neg] ? 1.0F : 0.0F);
    };
    bool isSprinting = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    f32 speed = isSprinting ? SPRINT_SPEED : SPEED;
    f32 deltaTime = deltaTimer.tick();
    Vec3<f32> horizontal = {
        .x = axis(SDL_SCANCODE_D, SDL_SCANCODE_A),
        .y = 0.0F,
        .z = axis(SDL_SCANCODE_S, SDL_SCANCODE_W)
    };
    if ((horizontal.x != 0.0F) || (horizontal.z != 0.0F)) {
        Vec3<f32> delta = horizontal.normalize() * speed * deltaTime;
        camera.adjust_position(delta, Space::VIEW);
        clearFilm = true;
    }
    Vec3<f32> vertical = {
        .x = 0.0F,
        .y = axis(SDL_SCANCODE_Q, SDL_SCANCODE_E),
        .z = 0.0F
    };
    if (vertical.y != 0.0F) {
        Vec3<f32> delta = vertical.normalize() * speed * deltaTime;
        camera.adjust_position(delta, Space::WORLD);
        clearFilm = true;
    }
    if (clearFilm) {
        film.clear();
    }
    return true;
}

int main() {
    try {
        Scene scene = make_scene();
        ThreadPool threadPool;
        Film film(WIDTH, HEIGHT, EXPOSURE);
        SdlContext context;
        SdlWindow window("Basic Pathtracing", WIDTH, HEIGHT);
        SdlDeltaTimer deltaTimer;
        SdlFpsCounter fpsCounter;
        while (handle_events_and_inputs(deltaTimer, scene.camera, film)) {
            cgfs::pathtrace(scene, threadPool, film);
            window.fill(film);
            if (std::optional<f64> fps = fpsCounter.tick()) {
                window.set_title(
                    std::format("Basic Pathtracing – {:.1F} FPS", *fps).c_str()
                );
            }
        }
    }
    catch (const std::exception& e) {
        std::println(stderr, "{}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}