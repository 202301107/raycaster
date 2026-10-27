#include "Renderer.h"

#include <algorithm>
#include <cstdio>

Renderer::Renderer(SDL_Renderer* sdl, int width, int height)
    : sdl_(sdl),
      texture_(nullptr),
      width_(width),
      height_(height),
      pixels_(static_cast<std::size_t>(width) * height, 0) {
    // STREAMING = a texture we rewrite from the CPU every frame.
    texture_ = SDL_CreateTexture(sdl_, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_) {
        std::fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
    }
}

Renderer::~Renderer() {
    if (texture_) SDL_DestroyTexture(texture_);
}

void Renderer::clear(std::uint32_t color) {
    // std::fill over the flat buffer; the compiler turns this into a fast
    // memset-like loop.
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void Renderer::drawVerticalLine(int x, int yTop, int yBottom,
                                std::uint32_t color) {
    if (x < 0 || x >= width_) return;         // column off-screen
    if (yTop > yBottom) return;               // empty span

    if (yTop < 0) yTop = 0;                    // clip to top edge
    if (yBottom >= height_) yBottom = height_ - 1;  // clip to bottom edge

    // Walk down the column. Index advances by width_ each row (next row, same
    // column), so we compute it once and add instead of multiplying per pixel.
    std::size_t index = static_cast<std::size_t>(yTop) * width_ + x;
    for (int y = yTop; y <= yBottom; ++y) {
        pixels_[index] = color;
        index += width_;
    }
}

void Renderer::setPixel(int x, int y, std::uint32_t color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
}

void Renderer::fillRect(int x, int y, int w, int h, std::uint32_t color) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w;
    int y1 = y + h;
    if (x1 > width_) x1 = width_;
    if (y1 > height_) y1 = height_;
    for (int yy = y0; yy < y1; ++yy)
        for (int xx = x0; xx < x1; ++xx)
            pixels_[static_cast<std::size_t>(yy) * width_ + xx] = color;
}

void Renderer::fillRows(int yTop, int yBottom, std::uint32_t color) {
    if (yTop < 0) yTop = 0;
    if (yBottom >= height_) yBottom = height_ - 1;
    if (yTop > yBottom) return;
    // Full-width rows form one contiguous block: a single sequential fill.
    const std::size_t begin = static_cast<std::size_t>(yTop) * width_;
    const std::size_t end = static_cast<std::size_t>(yBottom + 1) * width_;
    std::fill(pixels_.begin() + begin, pixels_.begin() + end, color);
}

void Renderer::present() {
    // Copy the whole framebuffer into the texture. The pitch (bytes per row) is
    // width * 4 for ARGB8888.
    SDL_UpdateTexture(texture_, nullptr, pixels_.data(),
                      width_ * static_cast<int>(sizeof(std::uint32_t)));
    SDL_RenderCopy(sdl_, texture_, nullptr, nullptr);
    SDL_RenderPresent(sdl_);
}

bool Renderer::savePPM(const char* path) const {
    std::FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    std::fprintf(f, "P6\n%d %d\n255\n", width_, height_);
    for (std::uint32_t px : pixels_) {
        unsigned char rgb[3] = {
            static_cast<unsigned char>((px >> 16) & 0xFF),  // R
            static_cast<unsigned char>((px >> 8) & 0xFF),   // G
            static_cast<unsigned char>(px & 0xFF),          // B
        };
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
    return true;
}
