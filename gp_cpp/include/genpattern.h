#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
// CFFI_BEGIN
typedef struct GPPoint {
  ptrdiff_t x, y;
} GPPoint;

typedef GPPoint GPVector;

typedef struct GPImgAlpha {
  size_t width;
  size_t height;
  uint8_t *data;
  //
  GPVector offsets[4];
  uint8_t offsets_size;
} GPImgAlpha;

typedef struct GPCollection {
  size_t n_images;
  GPImgAlpha *images;
} GPCollection;

const char *gp_genpattern(GPCollection *collections, const size_t n_collections,
                          const size_t canvas_width, const size_t canvas_height,
                          const uint8_t threshold, const size_t offset_radius,
                          const size_t collection_offset_radius);
// CFFI_END
}

#include "ImgAlpha.hpp"
#include "PatternGenerator.hpp"
#include <vector>

using namespace gp;

std::shared_ptr<ImgAlpha> init_ImgAlpha(std::vector<uint8_t> &data, const size_t width,
                        const size_t height, const uint8_t threshold);
PatternGenerator *init_PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<std::shared_ptr<ImgAlpha>>> &collections,
    const size_t offset, const size_t collection_offset);