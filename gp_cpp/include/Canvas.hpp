#pragma once

#include "BitImage.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>

namespace gp {
struct PlacementArea {
public:
  Box bounds;
  Point canvasStart;
  Point imageStart;
};

class Canvas : public BitImage {
private:
  enum Area { BOTTOM, RIGHT, BOTTOM_RIGHT, CANVAS, AREAS_SIZE };

  const std::array<Box, AREAS_SIZE> areas;
  const std::array<Vector, AREAS_SIZE> offsets;
  using BitImage::data;

  std::vector<PlacementArea> placementAreas(const BitImage &img,
                                            const Point pos) const {
    assert(img.getHeight() <= this->getHeight() &&
           img.getWidth() <= this->getWidth()); // TODO: exception

    std::vector<PlacementArea> out;
    out.reserve(4);

    Box bit_bounds = {pos, {pos.x + img.getWidth(), pos.y + img.getHeight()}};

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

  inline void processImageIntersection(
      const BitImage &img, const Point pos,
      std::function<void(bool &, const bool &)> action) const {
    const auto areas = this->placementAreas(img, pos);

    for (const auto &pa : areas) {
      for (ptrdiff_t i = pa.imageStart.y, ci = pa.canvasStart.y;
           i < pa.imageStart.y + pa.bounds.getHeight(); i++, ci++) {
        for (ptrdiff_t j = pa.imageStart.x, cj = pa.canvasStart.x;
             j < pa.imageStart.x + pa.bounds.getWidth(); j++, cj++) {
          action(this->data[ci, cj], img.data[i, j]);
        }
      }
    }
  }

  using nlp = std::numeric_limits<ptrdiff_t>;

public:
  Canvas(const ptrdiff_t width, const ptrdiff_t height)
      : BitImage(ImgAlpha(nullptr, width, height)),
        areas{{{{0, height}, {width, nlp::max()}},
               {{width, 0}, {nlp::max(), height}},
               {{width, height}, {nlp::max(), nlp::max()}},
               {{0, 0}, {width, height}}}},
        offsets{{{0, -height}, {-width, 0}, {-width, -height}, {0, 0}}} {}

  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  Canvas(Canvas &&other)
      : BitImage(static_cast<BitImage &&>(other)),
        areas(std::move(other.areas)), offsets(std::move(other.offsets)) {}

  ~Canvas() = default;

  void addImage(const BitImage &img, const Point pos) {
#ifndef NDEBUG
    const auto old_bits = this->nPixels();
    const auto img_bits = img.nPixels();
#endif

    this->processImageIntersection(img, pos,
                                   [](auto &canvasChunk, const auto &imgChunk) {
                                     canvasChunk |= imgChunk;
                                   });

#ifndef NDEBUG
    const auto new_bits = this->nPixels();
    std::cout << pos.x << " " << pos.y << "; old_bits " << old_bits
              << " new_bits " << new_bits << " imb " << img_bits << std::endl;
    assert(new_bits == old_bits + img_bits);
#endif
  }

  int intersectionArea(const BitImage &img, const Point pos) const {
    int res = 0;
    this->processImageIntersection(
        img, pos, [&res](auto &canvasChunk, const auto &imgChunk) {
          if (canvasChunk & imgChunk) {
            res++;
          }
        });
    return res;
  }
};

} // namespace gp
