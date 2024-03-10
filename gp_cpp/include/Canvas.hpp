#pragma once

#include "BitImage.hpp"
#include "ImgAlpha.hpp"

#include <bit>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

namespace gp {
struct PlacementArea {
 public:
  Box bounds;
  Point canvasStart;
  Point imageStart;
};

template <typename T>
class Canvas : public BitImage<T, false> {
 private:
  enum Area { BOTTOM, RIGHT, BOTTOM_RIGHT, CANVAS, AREAS_SIZE };

  const std::array<Box, AREAS_SIZE> areas;
  const aligned_mdarray<T, 2>& data;

  std::vector<PlacementArea> placementAreas(const BitImage<T>& img,
                                            const Point pos) const {
    assert(img.getHeight() <= this->getHeight() &&
           img.getBitWidth() <= this->getBitWidth());

    std::vector<PlacementArea> out;
    out.reserve(4);

    Box bit_bounds = {pos,
                      {pos.x + img.getBitWidth(), pos.y + img.getHeight()}};

    // TODO: move it...
    std::array<Vector, 4> offsets = {{}};
    offsets[CANVAS] = {0, 0};
    offsets[BOTTOM] = {0, -this->getHeight()};
    offsets[RIGHT] = {-this->getBitWidth(), 0};
    offsets[BOTTOM_RIGHT] = {-this->getBitWidth(), -this->getHeight()};

    for (int i = BOTTOM; i != AREAS_SIZE; i++) {
      Box intersection = bit_bounds.intersect(this->areas[i]);
      if (intersection.valid) {
        out.emplace_back(intersection.translate(offsets[i]),
                         intersection.min.translate(offsets[i]),
                         Point{intersection.min.x - bit_bounds.min.x,
                               intersection.min.y - bit_bounds.min.y});
        assert(out.size() < 4);
      }
    }

    return out;
  }

  inline void processImageIntersection(
      const BitImage<T>& img,
      const Point pos,
      std::function<void(T&, const T&)> action) const {
    const auto areas = this->placementAreas(img, pos);

    // TODO: non-divisible by BIT_SIZE canvas width support
    if (this->getBitWidth() % BIT_SIZE != 0) {
      throw std::runtime_error("Canvas width must be divisible by " +
                               std::to_string(BIT_SIZE));
    }

    for (const auto& pa : areas) {
      const auto view_idx = pa.canvasStart.x % BIT_SIZE;
      const auto& view = img.getOffsettedView(view_idx);

      const auto canvas_start_x = pa.canvasStart.x / BIT_SIZE;
      const auto byte_width =
          (pa.bounds.getWidth() + BIT_SIZE - 1) / BIT_SIZE;  // ceil

      for (ptrdiff_t i = pa.imageStart.y, ci = pa.canvasStart.y;
           i < pa.imageStart.y + pa.bounds.getHeight(); i++, ci++) {
        for (ptrdiff_t j = pa.imageStart.x, cj = canvas_start_x;
             j < pa.imageStart.x + byte_width; j++, cj++) {
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

  Canvas(const Canvas& other) = delete;
  Canvas& operator=(const Canvas&) = delete;

  Canvas(Canvas&& other)
      : BitImage<T, false>(static_cast<BitImage<T, false>&&>(other)),
        areas(std::move(other.areas)),
        data(other.data) {}

  ~Canvas() = default;

  using BitImage<T, false>::BIT_SIZE;

  void addImage(const BitImage<T>& img, const Point pos) {
    this->processImageIntersection(
        img, pos,
        [](T& canvasChunk, const T& imgChunk) { canvasChunk |= imgChunk; });
  }

  int intersectionArea(const BitImage<T>& img, const Point pos) const {
    int res = 0;
    this->processImageIntersection(img, pos,
                                   [&res](T& canvasChunk, const T& imgChunk) {
                                     const T p = canvasChunk & imgChunk;
                                     res += std::popcount(p);
                                   });
    return res;
  }
};

}  // namespace gp
