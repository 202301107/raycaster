// Wolfenstein-style raycaster — Milestone 1
// A clean SDL2 window with a fixed-timestep game loop.

#include <SDL.h>

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>

#include "Map.h"
#include "Player.h"
#include "Renderer.h"

namespace {

// Background colors. The screen splits at the vertical center (the horizon):
// ceiling above, floor below.
const std::uint32_t kCeilingColor = packColor(48, 48, 64);   // dim blue-gray
const std::uint32_t kFloorColor = packColor(72, 64, 56);     // dim brown

// Movement tuning (world units & radians per second; scaled by dt in update).
constexpr double kMoveSpeed = 3.0;         // walking/strafing speed
constexpr double kRotSpeed = 2.0;          // keyboard turn rate
constexpr double kMouseSensitivity = 0.0025;  // radians per pixel of mouse move

// Set from a signal handler when the user hits Ctrl-C (SIGINT) or the process
// is asked to terminate (SIGTERM). A handler may only touch a volatile
// sig_atomic_t safely, so we just flip this flag and react in the main loop,
// exercising the exact same clean-shutdown path as closing the window.
volatile std::sig_atomic_t g_quitRequested = 0;

extern "C" void handleTerminationSignal(int /*signo*/) { g_quitRequested = 1; }

constexpr int kWindowWidth = 1024;
constexpr int kWindowHeight = 768;

// The simulation advances in fixed 1/60-second steps. Every call to update()
// sees exactly this dt, which makes the simulation deterministic and stable.
constexpr double kFixedDt = 1.0 / 60.0;

// If a single frame takes longer than this (a stall, a breakpoint, a window
// drag), we pretend only this much time passed. Without the clamp, a long
// frame would queue up dozens of update() calls, each of which takes time,
// which makes the next frame longer still: the "spiral of death".
constexpr double kMaxFrameTime = 0.25;

// Tiny RAII guard so SDL_Quit() runs no matter how we leave main() — including
// early returns on error. Real engines wrap each subsystem like this.
struct SdlContext {
    bool ok = false;
    explicit SdlContext(Uint32 flags) { ok = (SDL_Init(flags) == 0); }
    ~SdlContext() {
        if (ok) SDL_Quit();
    }
    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;
};

// Advance the simulation by one fixed step. Handles continuous (held-key)
// input: WASD movement and arrow-key rotation, all scaled by dt so speed is
// framerate-independent. Mouse rotation is applied once per frame in the main
// loop instead (a mouse delta is not a rate). No collision yet — the player
// can walk through walls until Milestone 8.
void update(double dt, Player& player, const Uint8* keys) {
    const double move = kMoveSpeed * dt;
    const double rot = kRotSpeed * dt;

    if (keys[SDL_SCANCODE_W]) player.moveForward(move);
    if (keys[SDL_SCANCODE_S]) player.moveForward(-move);
    if (keys[SDL_SCANCODE_A]) player.strafe(-move);
    if (keys[SDL_SCANCODE_D]) player.strafe(move);
    if (keys[SDL_SCANCODE_LEFT]) player.rotate(rot);    // turn left (CCW)
    if (keys[SDL_SCANCODE_RIGHT]) player.rotate(-rot);  // turn right (CW)
}

// Paint the ceiling (top half) and floor (bottom half), split at the horizon.
// Walls will be drawn on top of this in later milestones.
void drawBackground(Renderer& renderer) {
    const int w = renderer.width();
    const int h = renderer.height();
    const int horizon = h / 2;

    for (int x = 0; x < w; ++x) {
        renderer.drawVerticalLine(x, 0, horizon - 1, kCeilingColor);
        renderer.drawVerticalLine(x, horizon, h - 1, kFloorColor);
    }
}

// Debug top-down overlay: the grid, the player's position (red) and facing
// direction (yellow). Toggled with Tab. Invaluable for sanity-checking the
// camera before and during the raycasting milestones.
void drawMinimap(Renderer& renderer, const Map& map, const Player& player) {
    constexpr int cell = 8;  // pixels per map cell
    constexpr int ox = 8, oy = 8;

    renderer.fillRect(ox - 2, oy - 2, map.width() * cell + 4,
                      map.height() * cell + 4, packColor(0, 0, 0));

    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            const std::uint32_t c = map.isWall(x, y) ? packColor(200, 200, 210)
                                                     : packColor(40, 40, 48);
            renderer.fillRect(ox + x * cell, oy + y * cell, cell - 1, cell - 1, c);
        }
    }

    // Player position. posX is the column, posY the row (matches map[y][x]).
    const int px = ox + static_cast<int>(player.posX * cell);
    const int py = oy + static_cast<int>(player.posY * cell);
    renderer.fillRect(px - 2, py - 2, 4, 4, packColor(230, 40, 40));

    // Facing direction, sampled as a short ray of pixels.
    for (double t = 0; t < cell * 3; t += 1.0) {
        renderer.setPixel(px + static_cast<int>(player.dirX * t),
                          py + static_cast<int>(player.dirY * t),
                          packColor(240, 220, 40));
    }
}

void render(Renderer& renderer, const Map& map, const Player& player,
            bool showMinimap) {
    drawBackground(renderer);
    // (walls come in Milestone 5/6; the camera is ready)
    if (showMinimap) drawMinimap(renderer, map, player);
    renderer.present();
}

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    std::signal(SIGINT, handleTerminationSignal);
    std::signal(SIGTERM, handleTerminationSignal);

    SdlContext sdl(SDL_INIT_VIDEO);
    if (!sdl.ok) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Raycaster",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWindowWidth, kWindowHeight,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    // Hardware-accelerated renderer. VSync is OFF deliberately: later we want to
    // measure true frame rate, not a number pinned to the monitor's refresh.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    Renderer framebuffer(renderer, kWindowWidth, kWindowHeight);
    const Map map = Map::createDefault();
    Player player = makeDefaultPlayer();
    bool showMinimap = true;

    // Relative mouse mode: hide the cursor and report motion as deltas so the
    // player can turn continuously (mouse-look). Harmless if it fails headless.
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Pointer to SDL's keyboard-state array; stays valid for the program's
    // lifetime and is refreshed by SDL_PollEvent/PumpEvents each frame.
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Dev/CI aid: if RAYCASTER_SCREENSHOT is set, render a single frame, dump it
    // to that PPM path, and exit. Lets us verify rendering headlessly.
    const char* screenshotPath = std::getenv("RAYCASTER_SCREENSHOT");

    // --- Fixed-timestep loop state ------------------------------------------
    // SDL_GetPerformanceCounter() is a high-resolution monotonic tick count;
    // dividing a delta by the frequency gives seconds.
    const Uint64 perfFreq = SDL_GetPerformanceFrequency();
    Uint64 previousTicks = SDL_GetPerformanceCounter();
    double accumulator = 0.0;  // real time waiting to be simulated

    bool running = true;
    while (running) {
        if (g_quitRequested) running = false;  // Ctrl-C / SIGTERM → clean exit

        // 1) Measure real time elapsed since the previous frame.
        const Uint64 currentTicks = SDL_GetPerformanceCounter();
        double frameTime =
            static_cast<double>(currentTicks - previousTicks) / perfFreq;
        previousTicks = currentTicks;

        if (frameTime > kMaxFrameTime) frameTime = kMaxFrameTime;  // spiral guard
        accumulator += frameTime;

        // 2) Input / window events, once per rendered frame. Mouse motion is
        //    accumulated here and applied once below (a delta, not a rate).
        double mouseDeltaX = 0.0;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (event.key.keysym.sym == SDLK_TAB) showMinimap = !showMinimap;
            } else if (event.type == SDL_MOUSEMOTION) {
                mouseDeltaX += event.motion.xrel;
            }
        }

        // 3) Drain the accumulator in fixed bites. Each update() sees kFixedDt.
        while (accumulator >= kFixedDt) {
            update(kFixedDt, player, keys);
            accumulator -= kFixedDt;
        }

        // Mouse-look: apply the frame's accumulated motion once. Moving the
        // mouse right (positive xrel) turns right, i.e. a negative rotation.
        if (mouseDeltaX != 0.0) player.rotate(-mouseDeltaX * kMouseSensitivity);

        // 4) Render. The leftover fraction of a step we haven't simulated:
        //    use it later as an interpolation alpha for perfectly smooth
        //    motion. Unused for now, hence the cast to void.
        const double alpha = accumulator / kFixedDt;
        (void)alpha;
        render(framebuffer, map, player, showMinimap);

        if (screenshotPath) {
            if (!framebuffer.savePPM(screenshotPath)) {
                std::fprintf(stderr, "Failed to write screenshot: %s\n",
                             screenshotPath);
            }
            running = false;  // one frame then quit
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
