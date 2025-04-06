#include "genpattern.h"

#include "CoolingSchedules.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "PatternGenerator.hpp"
#include <exception>
#include <stdexcept>

extern "C" {

GP_API int gp_genpattern(GPCollection *collections, const size_t n_collections,
                         const size_t canvas_width, const size_t canvas_height,
                         const uint8_t threshold, const size_t offset_radius,
                         const size_t collection_offset_radius,
                         const GPSchedule *const schedule, const uint32_t seed,
                         char *exception_text_buffer,
                         const size_t exception_text_buffer_size) {
  using namespace gp;

  std::vector<std::vector<ImgAlphaFilledContour>> collections_v(n_collections);

  for (size_t i = 0; i < n_collections; i++) {
    auto &collection = collections_v[i];
    collection.reserve(collections[i].n_images);
    for (size_t img_idx = 0; img_idx < collections[i].n_images; img_idx++) {
      const auto &img = collections[i].images[img_idx];
      collection.emplace_back(img.data, img.width, img.height, threshold);
    }
  }

  PatternGenerator pg(canvas_width, canvas_height, collections_v, offset_radius,
                      collection_offset_radius, 100.0);

  std::vector<std::vector<std::vector<Point>>> result;
  try {
    switch (schedule->type) {
    case GP_SCHEDULE_EXPONENTIAL:
      result = pg.generate(
          seed, ExponentialSchedule(schedule->params.exponential.alpha));
      break;
    case GP_SCHEDULE_LINEAR:
      result = pg.generate(seed, LinearSchedule(schedule->params.linear.k));
      break;
    default:
      throw std::invalid_argument("Unknown schedule type");
    }
  } catch (std::exception &e) {
    const char *what = e.what();
    if (exception_text_buffer == nullptr || exception_text_buffer_size == 0) {
      std::cerr << "[libgenpattern] Exception: " << what << std::endl;
      return -1;
    }
    const size_t size =
        std::min(exception_text_buffer_size - 1, std::strlen(what));
    std::memcpy(exception_text_buffer, what, size);
    exception_text_buffer[size] = '\0';
    return -1;
  }

  for (size_t col_idx = 0; col_idx < n_collections; col_idx++) {
    auto &collection = collections[col_idx];
    for (size_t img_idx = 0; img_idx < collection.n_images; img_idx++) {
      const auto &r = result[col_idx][img_idx];
      auto &img = collection.images[img_idx];
      img.offsets_size = r.size();
      for (uint8_t i = 0; i < img.offsets_size; i++) {
        img.offsets[i] = {r[i].getX(), r[i].getY()};
      }
    }
  }
  return 0;
}
}

#if __has_include(<emscripten/bind.h>)

using namespace gp;

std::shared_ptr<ImgAlphaFilledContour>
init_ImgAlphaFilledContour(std::vector<uint8_t> &data, const size_t width,
                           const size_t height, const uint8_t threshold) {
  return std::make_shared<ImgAlphaFilledContour>(data.data(), width, height,
                                                 threshold);
}

std::vector<std::vector<ImgAlphaFilledContour>> convertToImgAlphaVec(
    const std::vector<std::vector<std::shared_ptr<ImgAlphaFilledContour>>>
        &src) {
  std::vector<std::vector<ImgAlphaFilledContour>> dest;
  dest.reserve(src.size());

  for (const auto &innerVec : src) {
    std::vector<ImgAlphaFilledContour> tempVec;
    tempVec.reserve(innerVec.size());

    for (const auto &imgPtr : innerVec) {
      if (!imgPtr) {
        throw std::invalid_argument(
            "Null pointer encountered in source vector");
      }
      // Move the ImgAlpha object from the shared_ptr to the new vector
      tempVec.push_back(std::move(*imgPtr));
    }

    dest.push_back(std::move(tempVec));
  }

  return dest;
}

std::shared_ptr<PatternGenerator> init_PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<std::shared_ptr<ImgAlphaFilledContour>>>
        &collections,
    const size_t offset, const size_t collection_offset,
    const double temperatureInitial) {
  return std::make_shared<PatternGenerator>(
      width, height, convertToImgAlphaVec(collections), offset,
      collection_offset, temperatureInitial);
}

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(genpattern) {
  register_vector<uint8_t>("Uint8Vector");

  class_<ImgAlphaFilledContour>("ImgAlphaFilledContour")
      .smart_ptr_constructor("ImgAlphaFilledContour",
                             &init_ImgAlphaFilledContour);

  register_vector<std::shared_ptr<ImgAlphaFilledContour>>("Collection");

  register_vector<std::vector<std::shared_ptr<ImgAlphaFilledContour>>>(
      "CollectionVector");

  class_<Point>("Point")
      .constructor<const ptrdiff_t, const ptrdiff_t>()
      .property("x", &Point::getX)
      .property("y", &Point::getY);

  register_vector<Point>("PointVector");

  register_vector<std::vector<Point>>("PointVectorVector");

  register_vector<std::vector<std::vector<Point>>>("PointVectorVectorVector");

  value_object<ExponentialSchedule>("schedule_exponential")
      .field("alpha", &ExponentialSchedule::alpha);

  value_object<LinearSchedule>("schedule_linear")
      .field("k", &LinearSchedule::k);

  class_<PatternGenerator>("PatternGenerator")
      .smart_ptr_constructor("PatternGenerator", &init_PatternGenerator)
      .function("generate_exponential",
                &PatternGenerator::generate<ExponentialSchedule>)
      .function("generate_linear", &PatternGenerator::generate<LinearSchedule>);
}

#endif