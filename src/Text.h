// Minimal software text: a hand-coded 3x5 bitmap font drawn as scaled rects.
// Supports digits, 'F' 'P' 'S', space, '.' and ':' — enough for an FPS readout.
#ifndef RAYCASTER_TEXT_H
#define RAYCASTER_TEXT_H

#include <cstdint>
#include <string>

#include "Renderer.h"

// Draw `text` with its top-left at (x, y); each font pixel is `scale` x `scale`
// screen pixels. Unknown characters render as blank.
void drawText(Renderer& renderer, const std::string& text, int x, int y,
              int scale, std::uint32_t color);

#endif  // RAYCASTER_TEXT_H
