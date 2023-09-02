#pragma once

#include "BitImage.hpp"
#include "ImgAlpha.hpp"

#include <bit>
#include <cstddef>
#include <optional>
#include <stdexcept>

namespace gp {
template <typename T> class Canvas : public BitImage<T, false> {
private:
  enum Area { BOTTOM, RIGHT, BOTTOM_RIGHT, CANVAS, AREAS_SIZE };

  const std::array<Box, AREAS_SIZE> areas;
  const aligned_mdarray<T, 2> &data;

  size_t placementAreas(const BitImage<T> &img, const Point pos,
                        std::vector<std::pair<Point, Box>> &out) const {
    assert(out.size() >= 4);
    assert(img.getHeight() <= this->getHeight() &&
           img.getBitWidth() <= this->getBitWidth());

    size_t out_size = 0;

    Box bit_bounds = {pos,
                      {pos.x + img.getBitWidth(), pos.y + img.getHeight()}};

    for (int i = BOTTOM; i != AREAS_SIZE; i++) {
      Box intersection = bit_bounds.intersect(areas[i]);
      if (intersection.valid) {
        const Box image_area = intersection.translate(Vector{-pos.x, -pos.y});
        Point canvas_start_point;

        switch (i) {
        case CANVAS:
          canvas_start_point = pos;
          break;
        case BOTTOM:
          canvas_start_point = {pos.x, 0};
          break;
        case RIGHT:
          canvas_start_point = {0, pos.y};
          break;
        case BOTTOM_RIGHT:
          canvas_start_point = {0, 0};
          break;
        }

        out[out_size++] = std::make_pair(canvas_start_point, image_area);
      }
    }

    return out_size;
  }

  inline void
  processImageIntersection(const BitImage<T> &img, const Point pos,
                           std::function<void(T &, const T &)> action) const {
    std::vector<std::pair<Point, Box>> _areas(4);
    auto n_areas = this->placementAreas(img, pos, _areas);
    const auto areas = std::span(&_areas.front(), n_areas);

    // TODO: non-divisible by BIT_SIZE canvas width support
    if (this->getBitWidth() % BIT_SIZE != 0) {
      throw std::runtime_error("Canvas width must be divisible by " + std::to_string(BIT_SIZE));
    }

    /*
    for (int i = 0; i < n_areas; i++) {

      std::cout << "min x: " << areas[i].second.min.x
                << " y: " << areas[i].second.min.y << std::endl;
      std::cout << "max x: " << areas[i].second.max.x
                << " y: " << areas[i].second.max.y << std::endl
                << "x: " << areas[i].first.x << " y: " << areas[i].first.y
                << std::endl
                << std::endl;
    }*/

    // TODO: fix ugly varialbles naming
    for (auto [canvas_start_point, image_area] : areas) {
      const ptrdiff_t img_offset_x = -(image_area.min.x % BIT_SIZE);
      const ptrdiff_t canvas_offset_x = canvas_start_point.x % BIT_SIZE;
      ptrdiff_t canvas_start_x = canvas_start_point.x / BIT_SIZE;

      ptrdiff_t offset_x = img_offset_x + canvas_offset_x;
      ptrdiff_t img_start_x = image_area.min.x / BIT_SIZE;

      while (offset_x < 0) { // TODO: it may can be optimized
        offset_x += BIT_SIZE;
        img_start_x++;
      }

      const auto &view = img.getOffsettedView(offset_x);

      if (canvas_start_x < 0) {
        image_area.min.x -= canvas_start_x;
        img_start_x -= canvas_start_x;
        canvas_start_x = 0;
      }
      if (canvas_start_point.y < 0) {
        image_area.min.y -= canvas_start_point.y;
        canvas_start_point.y = 0;
      }

      assert(image_area.min.y >= 0);
      assert(canvas_start_point.y >= 0);
      assert(image_area.min.x >= 0);
      assert(canvas_start_x >= 0);

      for (ptrdiff_t i = image_area.min.y, ci = canvas_start_point.y;
           i < image_area.max.y; i++, ci++) {
        for (ptrdiff_t j = img_start_x, cj = canvas_start_x,
                       bj = image_area.min.x - canvas_offset_x;
             bj < image_area.max.x; j++, cj++, bj += BIT_SIZE) {
          action(data[ci, cj], view[i, j]);
        }
      }
    }
  }

  using nlp = std::numeric_limits<ptrdiff_t>;

public:
  Canvas(const ptrdiff_t width, const ptrdiff_t height)
      : BitImage<T, false>(ImgAlpha(nullptr, width, height)),
        data(BitImage<T, false>::data[0]),
        areas{{{{0, height}, {width, nlp::max()}},
               {{width, 0}, {nlp::max(), height}},
               {{width, height}, {nlp::max(), nlp::max()}},
               {{0, 0}, {width, height}}}} {}

  Canvas(const Canvas &other) = delete;
  Canvas &operator=(const Canvas &) = delete;

  Canvas(Canvas &&other)
      : BitImage<T, false>(static_cast<BitImage<T, false> &&>(other)),
        areas(std::move(other.areas)), data(other.data) {}

  ~Canvas() = default;

  using BitImage<T, false>::BIT_SIZE;

  void addImage(const BitImage<T> &img, const Point pos) {
    this->processImageIntersection(
        img, pos,
        [](T &canvasChunk, const T &imgChunk) { canvasChunk |= imgChunk; });
  }

  int intersectionArea(const BitImage<T> &img, const Point pos) const {
    int res = 0;
    this->processImageIntersection(img, pos,
                                   [&res](T &canvasChunk, const T &imgChunk) {
                                     const T p = canvasChunk & imgChunk;
                                     res += std::popcount(p);
                                   });
    return res;
  }
};

} // namespace gp
