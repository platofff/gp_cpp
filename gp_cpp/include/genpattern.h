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

typedef enum GPScheduleType { GP_EXPONENTIAL, GP_LINEAR } GPScheduleType;

typedef struct GPExponentialScheduleParams {
  double alpha;
} GPExponentialScheduleParams;

typedef struct GPLinearScheduleParams {
  double k;
} GPLinearScheduleParams;

typedef struct GPSchedule {
  GPScheduleType type;

  union {
    GPExponentialScheduleParams exponential;
    GPLinearScheduleParams linear;
  } params;
} GPSchedule;

const char *gp_genpattern(GPCollection *collections, const size_t n_collections,
                          const size_t canvas_width, const size_t canvas_height,
                          const uint8_t threshold, const size_t offset_radius,
                          const size_t collection_offset_radius,
                          const GPSchedule *const schedule, const uint32_t seed);
// CFFI_END
}

#if __has_include(<emscripten/bind.h>)

#include "PatternGenerator.hpp"
#include <vector>

using namespace gp;

std::shared_ptr<ImgAlphaFilledContour> init_ImgAlphaFilledContour(
    std::vector<uint8_t> &data, const size_t width, const size_t height,
    const uint8_t threshold);
std::shared_ptr<PatternGenerator> init_PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<std::shared_ptr<ImgAlphaFilledContour>>>
        &collections,
    const size_t offset, const size_t collection_offset,
    const double temperatureInitial);

#endif