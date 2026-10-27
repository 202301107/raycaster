#include "Text.h"

namespace {

// Each glyph is 5 rows of 3 columns. A set bit (from the left) is a lit pixel.
struct Glyph {
    unsigned char rows[5];  // low 3 bits used, bit 2 = leftmost column
};

// Build a glyph from readable "xxx" row strings ('1'/'#' = lit).
constexpr unsigned char rowBits(const char* r) {
    return static_cast<unsigned char>(((r[0] != '0' && r[0] != ' ') << 2) |
                                      ((r[1] != '0' && r[1] != ' ') << 1) |
                                      ((r[2] != '0' && r[2] != ' ') << 0));
}

Glyph glyphFor(char c) {
    switch (c) {
        case '0': return {{rowBits("111"), rowBits("101"), rowBits("101"), rowBits("101"), rowBits("111")}};
        case '1': return {{rowBits("010"), rowBits("110"), rowBits("010"), rowBits("010"), rowBits("111")}};
        case '2': return {{rowBits("111"), rowBits("001"), rowBits("111"), rowBits("100"), rowBits("111")}};
        case '3': return {{rowBits("111"), rowBits("001"), rowBits("111"), rowBits("001"), rowBits("111")}};
        case '4': return {{rowBits("101"), rowBits("101"), rowBits("111"), rowBits("001"), rowBits("001")}};
        case '5': return {{rowBits("111"), rowBits("100"), rowBits("111"), rowBits("001"), rowBits("111")}};
        case '6': return {{rowBits("111"), rowBits("100"), rowBits("111"), rowBits("101"), rowBits("111")}};
        case '7': return {{rowBits("111"), rowBits("001"), rowBits("010"), rowBits("010"), rowBits("010")}};
        case '8': return {{rowBits("111"), rowBits("101"), rowBits("111"), rowBits("101"), rowBits("111")}};
        case '9': return {{rowBits("111"), rowBits("101"), rowBits("111"), rowBits("001"), rowBits("111")}};
        case 'F': return {{rowBits("111"), rowBits("100"), rowBits("111"), rowBits("100"), rowBits("100")}};
        case 'P': return {{rowBits("111"), rowBits("101"), rowBits("111"), rowBits("100"), rowBits("100")}};
        case 'S': return {{rowBits("111"), rowBits("100"), rowBits("111"), rowBits("001"), rowBits("111")}};
        case ':': return {{rowBits("000"), rowBits("010"), rowBits("000"), rowBits("010"), rowBits("000")}};
        case '.': return {{rowBits("000"), rowBits("000"), rowBits("000"), rowBits("000"), rowBits("010")}};
        default:  return {{0, 0, 0, 0, 0}};  // space / unknown
    }
}

}  // namespace

void drawText(Renderer& renderer, const std::string& text, int x, int y,
              int scale, std::uint32_t color) {
    int penX = x;
    for (char c : text) {
        const Glyph g = glyphFor(c);
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (g.rows[row] & (1 << (2 - col))) {
                    renderer.fillRect(penX + col * scale, y + row * scale,
                                      scale, scale, color);
                }
            }
        }
        penX += 4 * scale;  // 3-wide glyph + 1 column spacing
    }
}
