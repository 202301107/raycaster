#include "Texture.h"

#include <SDL.h>
#include <SDL_image.h>

#include <cstdio>

Texture loadTexture(const std::string& path) {
    Texture tex;

    SDL_Surface* loaded = IMG_Load(path.c_str());
    if (!loaded) {
        std::fprintf(stderr, "IMG_Load(%s) failed: %s\n", path.c_str(),
                     IMG_GetError());
        return tex;  // invalid
    }

    // Convert whatever format the file used into our framebuffer format so each
    // texel is a direct ARGB8888 word.
    SDL_Surface* conv =
        SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(loaded);
    if (!conv) {
        std::fprintf(stderr, "ConvertSurface(%s) failed: %s\n", path.c_str(),
                     SDL_GetError());
        return tex;
    }

    tex.width = conv->w;
    tex.height = conv->h;
    tex.pixels.resize(static_cast<std::size_t>(conv->w) * conv->h);

    // Copy row by row: a surface's pitch (bytes per row) may exceed w*4 due to
    // padding, so we can't assume the buffer is tightly packed.
    SDL_LockSurface(conv);
    const auto* src = static_cast<const std::uint8_t*>(conv->pixels);
    for (int y = 0; y < conv->h; ++y) {
        const auto* row =
            reinterpret_cast<const std::uint32_t*>(src + y * conv->pitch);
        for (int x = 0; x < conv->w; ++x)
            tex.pixels[static_cast<std::size_t>(y) * conv->w + x] = row[x];
    }
    SDL_UnlockSurface(conv);
    SDL_FreeSurface(conv);

    return tex;
}
