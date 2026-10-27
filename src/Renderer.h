// Software framebuffer renderer.
//
// Owns a CPU-side pixel buffer that we draw into directly (one vertical strip
// per screen column, the raycaster's natural output), then uploads to an SDL
// streaming texture and blits it to the window once per frame.
#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

#include <SDL.h>

#include <cstdint>
#include <vector>

// Pack 8-bit channels into an ARGB8888 word (0xAARRGGBB). SDL's pixel-format
// names are logical, so this is correct regardless of host endianness.
constexpr std::uint32_t packColor(std::uint8_t r, std::uint8_t g,
                                  std::uint8_t b, std::uint8_t a = 255) {
    return (static_cast<std::uint32_t>(a) << 24) |
           (static_cast<std::uint32_t>(r) << 16) |
           (static_cast<std::uint32_t>(g) << 8) |
           (static_cast<std::uint32_t>(b));
}

class Renderer {
public:
    Renderer(SDL_Renderer* sdl, int width, int height);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Fill the entire framebuffer with one color.
    void clear(std::uint32_t color);

    // Draw a vertical span in column x, from row yTop to yBottom (inclusive).
    // Out-of-range columns are ignored; yTop/yBottom are clamped to the screen,
    // so callers may pass off-screen wall extents without extra checks.
    void drawVerticalLine(int x, int yTop, int yBottom, std::uint32_t color);

    // Write a single pixel; silently ignores out-of-bounds coordinates.
    void setPixel(int x, int y, std::uint32_t color);

    // Fill an axis-aligned rectangle; clipped to the framebuffer.
    void fillRect(int x, int y, int w, int h, std::uint32_t color);

    // Fill full rows yTop..yBottom (inclusive) with one color. This spans whole
    // rows, so the region is contiguous in memory and fills sequentially (one
    // cache line at a time) — far faster than the equivalent column-wise fill.
    void fillRows(int yTop, int yBottom, std::uint32_t color);

    // Upload the framebuffer to the GPU texture and present it to the window.
    void present();

    // Write the current framebuffer to a binary PPM (P6) file. A dev/CI aid so
    // output can be inspected headlessly. Returns false on I/O error.
    bool savePPM(const char* path) const;

    int width() const { return width_; }
    int height() const { return height_; }

private:
    SDL_Renderer* sdl_;       // not owned
    SDL_Texture* texture_;    // owned
    int width_;
    int height_;
    std::vector<std::uint32_t> pixels_;
};

#endif  // RAYCASTER_RENDERER_H
