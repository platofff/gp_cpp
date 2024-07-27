#include "genpattern.h"
#include "ImgAlpha.hpp"
#include "PatternGenerator.hpp"
#include <exception>

extern "C" {

const char *gp_genpattern(GPCollection *collections, const size_t n_collections,
                          const size_t canvas_width, const size_t canvas_height,
                          const uint8_t threshold, const size_t offset_radius,
                          const size_t collection_offset_radius) {
  using namespace gp;

  std::vector<std::vector<ImgAlpha>> collections_v(n_collections);

  for (size_t i = 0; i < n_collections; i++) {
    auto &collection = collections_v[i];
    collection.reserve(collections[i].n_images);
    for (size_t img_idx = 0; img_idx < collections[i].n_images; img_idx++) {
      const auto &img = collections[i].images[img_idx];
      collection.emplace_back(img.data, img.width, img.height);
      collection.back().generateAndFillContour(threshold);
      // std::cout << collection.back().getContour().size() << std::endl;
    }
  }

  PatternGenerator pg(canvas_width, canvas_height, collections_v, offset_radius,
                      collection_offset_radius);

  std::vector<std::vector<std::vector<Point>>> result;
  try {
    result = pg.generate();
  } catch (std::exception &e) {
    const char *what = e.what();
    const size_t buf_size = std::strlen(what) + 1;
    char *buf = static_cast<char *>(std::calloc(buf_size, sizeof(char)));
    if (buf == nullptr) {
      std::cerr << "Failed to allocate memory for exception message"
                << std::endl;
      std::abort();
    }
    std::memcpy(buf, what, buf_size);
    return buf;
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
  return nullptr;
}
}

#if __has_include(<emscripten/bind.h>)

using namespace gp;

std::shared_ptr<ImgAlpha> init_ImgAlpha(std::vector<uint8_t> &data, const size_t width,
                        const size_t height, const uint8_t threshold) {
  auto obj = std::make_shared<ImgAlpha>(data.data(), width, height);
  obj->generateAndFillContour(threshold);
  return obj;
}

std::vector<std::vector<ImgAlpha>> convertToImgAlphaVec(
    const std::vector<std::vector<std::shared_ptr<ImgAlpha>>> &src) {
  std::vector<std::vector<ImgAlpha>> dest;
  dest.reserve(src.size());

  for (const auto &innerVec : src) {
    std::vector<ImgAlpha> tempVec;
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

PatternGenerator *init_PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<std::shared_ptr<ImgAlpha>>> &collections,
    const size_t offset, const size_t collection_offset) {
  return new PatternGenerator{width, height, convertToImgAlphaVec(collections),
                              offset, collection_offset};
}

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(genpattern) {
  register_vector<uint8_t>("Uint8Vector");

  class_<ImgAlpha>("ImgAlpha")
    .smart_ptr_constructor("ImgAlpha", &init_ImgAlpha);

  register_vector<std::shared_ptr<ImgAlpha>>("Collection");

  register_vector<std::vector<std::shared_ptr<ImgAlpha>>>("CollectionVector");

  value_object<Point>("Point").field("x", &Point::x).field("y", &Point::y);

  register_vector<Point>("PointVector");

  register_vector<std::vector<Point>>("PointVectorVector");

  register_vector<std::vector<std::vector<Point>>>("PointVectorVectorVector");

  class_<PatternGenerator>("PatternGenerator")
      .constructor(&init_PatternGenerator)
      .function("generate", &PatternGenerator::generate);
}

#endif