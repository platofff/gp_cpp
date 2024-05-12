#include "genpattern.h"
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

  PatternGenerator pg(canvas_width, canvas_height, collections_v,
                               offset_radius, collection_offset_radius);

  std::vector<std::vector<std::optional<std::vector<Point>>>> result;
  try {
    result = pg.generate();
  } catch (std::exception &e) {
    const char *what = e.what();
    const size_t buf_size = std::strlen(what) + 1;
    char *buf =
        static_cast<char *>(std::calloc(buf_size, sizeof(char)));
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
      const auto &_r = result[col_idx][img_idx];
      auto &img = collection.images[img_idx];
      if (_r.has_value()) {
        const auto &r = _r.value();
        img.offsets_size = r.size();
        for (uint8_t i = 0; i < img.offsets_size; i++) {
          img.offsets[i].x = r[i].x;
          img.offsets[i].y = r[i].y;
        }
      } else {
        img.offsets_size = 0;
      }
    }
  }
  return nullptr;
}
}