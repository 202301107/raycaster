// A wall texture: pixels decoded from an image file into our ARGB8888 format,
// so sampling at render time is a plain array read.
#ifndef RAYCASTER_TEXTURE_H
#define RAYCASTER_TEXTURE_H

#include <cstdint>
#include <string>
#include <vector>

struct Texture {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;  // row-major ARGB8888

    bool valid() const { return width > 0 && height > 0; }
    std::uint32_t at(int x, int y) const {
        return pixels[static_cast<std::size_t>(y) * width + x];
    }
};

// Load an image (PNG, etc.) via SDL2_image, converting to ARGB8888. Returns an
// empty (invalid) Texture on failure so callers can fall back to flat color.
Texture loadTexture(const std::string& path);

#endif  // RAYCASTER_TEXTURE_H
