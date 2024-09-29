#include "Canvas.hpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace gp {
std::vector<PlacementArea> Canvas::placementAreas(const BitImage &img,
                                                  const Point pos) const {
  if (img.getHeight() > this->getHeight() ||
      img.getWidth() > this->getWidth()) {
    throw std::invalid_argument("An image is larger than the canvas");
  }

  std::vector<PlacementArea> out;
  out.reserve(4);

  const auto imgWidth = static_cast<ptrdiff_t>(img.getWidth());
  const auto imgHeight = static_cast<ptrdiff_t>(img.getHeight());
  Box bounds = {pos, {pos.getX() + imgWidth - 1, pos.getY() + imgHeight - 1}};
  // std::cout << bounds << std::endl;

  for (int i = BOTTOM; i != AREAS_SIZE; i++) {
    Box intersection = bounds.intersect(this->areas[i]);
    if (intersection.isValid()) {
      // std::cout << this->areas[i] << " " << intersection << std::endl;
      out.emplace_back(
          intersection.translate(this->offsets[i]),
          intersection.getMin().translate(this->offsets[i]),
          Point{intersection.getMin().getX() - bounds.getMin().getX(),
                intersection.getMin().getY() - bounds.getMin().getY()});
      // std::cout << out.back().imageStart << std::endl;
      assert(out.size() <= 4);
    }
  }

  return out;
}

Canvas::Canvas(const ptrdiff_t width, const ptrdiff_t height, std::mt19937 &rng)
    : BitImage(ImgAlpha(nullptr, width, height)),
      areas{{{{0, height}, {width - 1, nlp::max()}},
             {{width, 0}, {nlp::max(), height - 1}},
             {{width, height}, {nlp::max(), nlp::max()}},
             {{0, 0}, {width - 1, height - 1}}}},
      offsets{{{0, -height}, {-width, 0}, {-width, -height}, {0, 0}}},
      deltaMaxInitial{std::ceil(static_cast<double>(width) / 2.0),
                      std::ceil(static_cast<double>(height) / 2.0)},
      rng{rng} {}

Canvas::Canvas(Canvas &&other)
    : BitImage(static_cast<BitImage &&>(other)), areas(std::move(other.areas)),
      offsets(std::move(other.offsets)),
      deltaMaxInitial(std::move(other.deltaMaxInitial)), rng(other.rng) {}

void Canvas::addImage(const BitImage &img, const Point pos) {
  const auto areas = this->placementAreas(img, pos);

  for (const auto &pa : areas) {
    for (ptrdiff_t i = pa.imageStart.getY(), ci = pa.canvasStart.getY();
         i < pa.imageStart.getY() + pa.bounds.getHeight(); i++, ci++) {
      for (ptrdiff_t j = pa.imageStart.getX(), cj = pa.canvasStart.getX();
           j < pa.imageStart.getX() + pa.bounds.getWidth(); j++, cj++) {
        this->setPixel(ci, cj, (*this)[ci, cj] || img[i, j]);
      }
    }
  }
}

uint64_t Canvas::intersectionArea(const BitImage &img, const Point pos) const {
  uint64_t res = 0;
  const auto areas = this->placementAreas(img, pos);

  for (const auto &pa : areas) {
    for (ptrdiff_t i = pa.imageStart.getY(), ci = pa.canvasStart.getY();
         i < pa.imageStart.getY() + pa.bounds.getHeight(); i++, ci++) {
      for (ptrdiff_t j = pa.imageStart.getX(), cj = pa.canvasStart.getX();
           j < pa.imageStart.getX() + pa.bounds.getWidth(); j++, cj++) {
        if ((*this)[ci, cj] && img[i, j]) {
          res++;
        }
      }
    }
  }
  return res;
}

Point Canvas::wrapPosition(const ptrdiff_t x, const ptrdiff_t y) const {
  return Point{((x % this->getWidth()) + this->getWidth()) % this->getWidth(),
               ((y % this->getHeight()) + this->getHeight()) %
                   this->getHeight()};
};
} // namespace gp
