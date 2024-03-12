#pragma once

#include "BitImage.hpp"
#include "ImgAlpha.hpp"
#include "misc.hpp"

#include <bit>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

namespace gp {
struct PlacementArea {
public:
  Box bounds;
  Point canvasStart;
  Point imageStart;
};

template <typename T> class Canvas : public BitImage<T, false> {
private:
  enum Area { BOTTOM, RIGHT, BOTTOM_RIGHT, CANVAS, AREAS_SIZE };

  const std::array<Box, AREAS_SIZE> areas;
  const std::array<Vector, AREAS_SIZE> offsets;
  const aligned_mdarray<T, 2> &data;

  std::vector<PlacementArea> placementAreas(const BitImage<T> &img,
                                            const Point pos) const {
    assert(img.getHeight() <= this->getHeight() &&
           img.getBitWidth() <= this->getBitWidth()); // TODO: exception

    std::vector<PlacementArea> out;
    out.reserve(4);

    Box bit_bounds = {pos,
                      {pos.x + img.getBitWidth(), pos.y + img.getHeight()}};

    for (int i = BOTTOM; i != AREAS_SIZE; i++) {
      Box intersection = bit_bounds.intersect(this->areas[i]);
      if (intersection.valid) {
        out.emplace_back(intersection.translate(this->offsets[i]),
                         intersection.min.translate(this->offsets[i]),
                         Point{intersection.min.x - bit_bounds.min.x,
                               intersection.min.y - bit_bounds.min.y});
        assert(out.size() <= 4);
      }
    }

    return out;
  }

  inline void
  processImageIntersection(const BitImage<T> &img, const Point pos,
                           std::function<void(T &, const T &)> action) const {
    const auto areas = this->placementAreas(img, pos);

    // TODO: non-divisible by BIT_SIZE canvas width support
    if (this->getBitWidth() % BIT_SIZE != 0) {
      throw std::runtime_error("Canvas width must be divisible by " +
                               std::to_string(BIT_SIZE));
    }

    for (const auto &pa : areas) {
      const auto view_idx = positive_modulo(pa.canvasStart.x - pa.imageStart.x, BIT_SIZE); 
      const auto &view = img.getOffsettedView(view_idx);

      const auto canvas_start_x = pa.canvasStart.x / BIT_SIZE;
      const auto image_start_x = (pa.imageStart.x + view_idx) / BIT_SIZE;

      auto byte_width =
          ((pa.bounds.getWidth() + BIT_SIZE - view_idx + 1) + BIT_SIZE - 1) / BIT_SIZE; // ceil
      
      // TODO ?
      if (byte_width + image_start_x > img.getWidth()) {
        byte_width = img.getWidth() - image_start_x;
      }
      if (byte_width + canvas_start_x > this->getWidth()) {
        byte_width = this->getWidth() - canvas_start_x;
      }

      /*      
      std::cout << "Image start: " << pa.imageStart.x << " " << pa.imageStart.y
                << std::endl;
      std::cout << "Box height: " << pa.bounds.getHeight()
                << " Box width: " << pa.bounds.getWidth() << std::endl;
      std::cout << "Byte width: " << byte_width << std::endl;
      std::cout << "View idx: " << view_idx << std::endl;
      std::cout << std::endl;*/

      for (ptrdiff_t i = pa.imageStart.y, ci = pa.canvasStart.y;
           i < pa.imageStart.y + pa.bounds.getHeight(); i++, ci++) {
        for (ptrdiff_t j = image_start_x, cj = canvas_start_x;
             j < image_start_x + byte_width; j++, cj++) {
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
               {{0, 0}, {width, height}}}},
        offsets{{{0, -height}, {-width, 0}, {-width, -height}, {0, 0}}} {}

  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  Canvas(Canvas &&other)
      : BitImage<T, false>(static_cast<BitImage<T, false> &&>(other)),
        areas(std::move(other.areas)), data(other.data),
        offsets(std::move(other.offsets)) {}

  ~Canvas() = default;

  using BitImage<T, false>::BIT_SIZE;

  void addImage(const BitImage<T> &img, const Point pos) {
#ifndef NDEBUG
    const auto old_bits = this->nPixels();
    const auto img_bits = img.nPixels();
#endif

    this->processImageIntersection(
        img, pos,
        [](T &canvasChunk, const T &imgChunk) { canvasChunk |= imgChunk; });

#ifndef NDEBUG
    const auto new_bits = this->nPixels();
    std::cout << pos.x << " " << pos.y  << "; old_bits " << old_bits << " new_bits " << new_bits << " imb " << img_bits << std::endl;
    //assert(new_bits == old_bits + img_bits);
#endif
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
