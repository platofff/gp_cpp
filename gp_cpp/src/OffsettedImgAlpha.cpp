#include "OffsettedImgAlpha.hpp"
#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"

#include <cassert>
#include <limits>
#include <unordered_set>

namespace gp {
const aligned_mdarray<uint8_t, 2>
OffsettedImgAlpha::generateDisk(const ptrdiff_t r) {
  const size_t n = 2 * r + 1;
  auto disk = make_aligned_mdarray<uint8_t>(n, n);

  for (ptrdiff_t i = 0; i < n; i++) {
    for (ptrdiff_t j = 0; j < n; j++) {
      const auto x = static_cast<double>(j - r);
      const auto y = static_cast<double>(i - r);
      const auto d = std::sqrt(x * x + y * y);
      if (d <= static_cast<double>(r)) {
        disk[i, j] = ImgAlpha::FILL_VALUE;
      }
    }
  }

  return std::move(disk);
}

std::pair<const Box, const std::unordered_set<Point>>
OffsettedImgAlpha::minkowskiSum(const std::vector<Point> &contour,
                                const aligned_mdarray<uint8_t, 2> &disk,
                                const ptrdiff_t r) {
  using nlp = std::numeric_limits<ptrdiff_t>;
  ptrdiff_t min_x = nlp::max();
  ptrdiff_t min_y = nlp::max();
  ptrdiff_t max_x = nlp::min();
  ptrdiff_t max_y = nlp::min();
  std::unordered_set<Point> points_to_fill;

  for (const auto &p : contour) {
    const Point pos{p.getX() - r, p.getY() - r};
    for (ptrdiff_t i = 0; i < disk.extent(0); i++) {
      for (ptrdiff_t j = 0; j < disk.extent(1); j++) {
        if (disk[i, j] != ImgAlpha::FILL_VALUE) {
          continue;
        }
        const auto x = pos.getX() + j;
        const auto y = pos.getY() + i;
        const auto [_, inserted] = points_to_fill.insert(Point{x, y});
        if (inserted) {
          min_x = std::min(min_x, x);
          min_y = std::min(min_y, y);
          max_x = std::max(max_x, x);
          max_y = std::max(max_y, y);
        }
      }
    }
  }

  const Box bounds{{min_x, min_y}, {max_x, max_y}};
  return std::make_pair(bounds, points_to_fill);
}

void OffsettedImgAlpha::fill(const Box &bounds,
                             const std::unordered_set<Point> &points_to_fill,
                             const ImgAlpha &img) {
  const size_t width = bounds.getWidth();
  const size_t height = bounds.getHeight();

  this->alpha = make_aligned_mdarray<uint8_t>(height, width);

  for (const auto &point : points_to_fill) {
    this->alpha[point.getY() - baseOffset.getY(),
                point.getX() - baseOffset.getX()] = ImgAlpha::FILL_VALUE;
  }

  for (ptrdiff_t i = 0; i < img.getHeight(); i++) {
    for (ptrdiff_t j = 0; j < img.getWidth(); j++) {
      if (img[i, j] == ImgAlpha::FILL_VALUE) {
        this->alpha[i - baseOffset.getY(), j - baseOffset.getX()] =
            ImgAlpha::FILL_VALUE;
      }
    }
  }
}

OffsettedImgAlpha::OffsettedImgAlpha(const ImgAlphaFilledContour &img, const ptrdiff_t r)
    : baseOffset(0, 0) {
  const auto &contour = img.getContour();
  assert(!contour.empty());

  const auto disk = this->generateDisk(r);
  const auto [bounds, points_to_fill] = this->minkowskiSum(contour, disk, r);

  this->baseOffset = bounds.getMin();

  this->fill(bounds, points_to_fill, img);
}

Vector OffsettedImgAlpha::getBaseOffset() const { return this->baseOffset; }
} // namespace gp