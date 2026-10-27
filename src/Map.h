// The level: a fixed 2D grid of unit cells.
//
// Each cell is an integer: 0 = empty, nonzero = a solid wall (the value will
// later select a texture/color). One cell spans 1.0 x 1.0 in world space, so
// the world cell at a floating-point position (wx, wy) is (int)wx, (int)wy.
#ifndef RAYCASTER_MAP_H
#define RAYCASTER_MAP_H

#include <vector>

class Map {
public:
    // Build the hardcoded 24x24 demo level.
    static Map createDefault();

    int width() const { return width_; }
    int height() const { return height_; }

    // Cell value at grid (x, y). Out-of-bounds is reported as a wall so rays
    // can never run past the array and the DDA loop always terminates.
    int at(int x, int y) const {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return 1;
        return cells_[static_cast<std::size_t>(y) * width_ + x];
    }

    bool isWall(int x, int y) const { return at(x, y) != 0; }

private:
    Map(int width, int height, std::vector<int> cells)
        : width_(width), height_(height), cells_(std::move(cells)) {}

    int width_;
    int height_;
    std::vector<int> cells_;  // row-major: cells_[y * width_ + x]
};

#endif  // RAYCASTER_MAP_H
