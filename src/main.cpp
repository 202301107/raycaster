// Wolfenstein-style raycaster — Milestone 1
// A clean SDL2 window with a fixed-timestep game loop.

#include <SDL.h>

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>

#include "Renderer.h"

namespace {

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

// Advance the game simulation by one fixed step. Empty for now; the player and
// world arrive in later milestones. The important thing is that it *only* ever
// receives kFixedDt.
void update(double /*dt*/) {
    // (nothing to simulate yet)
}

// TEMPORARY Milestone-2 demo: prove the vertical-line helper works by drawing
// variable-height color bars across the screen. Replaced by the raycaster in
// later milestones.
void render(Renderer& renderer) {
    renderer.clear(packColor(24, 24, 32));  // dark background

    const int w = renderer.width();
    const int h = renderer.height();
    const int cy = h / 2;

    for (int x = 0; x < w; ++x) {
        // Height sweeps smoothly across the screen so we can see clipping at
        // the extremes and correct per-column fills everywhere.
        const double t = static_cast<double>(x) / w;          // 0..1
        const int half = static_cast<int>((0.1 + 0.6 * std::sin(t * 3.14159)) * h);

        // Color cycles with x so adjacent columns are distinguishable.
        const auto r = static_cast<std::uint8_t>(128 + 127 * std::sin(t * 6.283));
        const auto g = static_cast<std::uint8_t>(128 + 127 * std::sin(t * 6.283 + 2.0));
        const auto b = static_cast<std::uint8_t>(128 + 127 * std::sin(t * 6.283 + 4.0));

        renderer.drawVerticalLine(x, cy - half, cy + half, packColor(r, g, b));
    }

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

        // 2) Input / window events, once per rendered frame.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        // 3) Drain the accumulator in fixed bites. Each update() sees kFixedDt.
        while (accumulator >= kFixedDt) {
            update(kFixedDt);
            accumulator -= kFixedDt;
        }

        // 4) Render. The leftover fraction of a step we haven't simulated:
        //    use it later as an interpolation alpha for perfectly smooth
        //    motion. Unused for now, hence the cast to void.
        const double alpha = accumulator / kFixedDt;
        (void)alpha;
        render(framebuffer);

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
