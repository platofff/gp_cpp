#pragma once

#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#define GP_API __declspec(dllexport)
#else
#define GP_API __attribute__((visibility("default")))
#endif

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

const uint8_t GP_SCHEDULE_EXPONENTIAL = 0x00;
const uint8_t GP_SCHEDULE_LINEAR = 0x01;

typedef struct GPExponentialScheduleParams {
  double alpha;
} GPExponentialScheduleParams;

typedef struct GPLinearScheduleParams {
  double k;
} GPLinearScheduleParams;

typedef struct GPSchedule {
  uint8_t type; // must be on of GP_SCHEDULE_* constants

  union {
    GPExponentialScheduleParams exponential;
    GPLinearScheduleParams linear;
  } params;
} GPSchedule;

GP_API int gp_genpattern(GPCollection *collections, const size_t n_collections,
                         const size_t canvas_width, const size_t canvas_height,
                         const uint8_t threshold, const size_t offset_radius,
                         const size_t collection_offset_radius,
                         const GPSchedule *const schedule, const uint32_t seed,
                         char *exception_text_buffer,
                         const size_t exception_text_buffer_size);
// CFFI_END
}

#if __has_include(<emscripten/bind.h>)

#include "PatternGenerator.hpp"
#include <vector>

using namespace gp;

std::shared_ptr<ImgAlphaFilledContour>
init_ImgAlphaFilledContour(std::vector<uint8_t> &data, const size_t width,
                           const size_t height, const uint8_t threshold);
std::shared_ptr<PatternGenerator> init_PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<std::shared_ptr<ImgAlphaFilledContour>>>
        &collections,
    const size_t offset, const size_t collection_offset,
    const double temperatureInitial);

#endif