#include "OffsettedImgAlpha.hpp"
#include "ImgAlpha.hpp"
#include "misc.hpp"

#include <limits>
#include <unordered_set>
#include <cassert>

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
  Box bounds{{std::numeric_limits<ptrdiff_t>::max(),
              std::numeric_limits<ptrdiff_t>::max()},
             {std::numeric_limits<ptrdiff_t>::min(),
              std::numeric_limits<ptrdiff_t>::min()}};
  std::unordered_set<Point> points_to_fill;

  for (const auto &p : contour) {
    const Point pos{p.x - r, p.y - r};
    for (ptrdiff_t i = 0; i < disk.extent(0); i++) {
      for (ptrdiff_t j = 0; j < disk.extent(1); j++) {
        if (disk[i, j] != ImgAlpha::FILL_VALUE) {
          continue;
        }
        const auto x = pos.x + j;
        const auto y = pos.y + i;
        const auto [_, inserted] = points_to_fill.insert(Point{x, y});
        if (inserted) {
          bounds.max.x = std::max(bounds.max.x, x);
          bounds.min.x = std::min(bounds.min.x, x);
          bounds.max.y = std::max(bounds.max.y, y);
          bounds.min.y = std::min(bounds.min.y, y);
        }
      }
    }
  }

  return std::make_pair(bounds, points_to_fill);
}

void OffsettedImgAlpha::fill(const Box &bounds,
                             const std::unordered_set<Point> &points_to_fill,
                             const ImgAlpha &img) {
  const size_t width = bounds.max.x - bounds.min.x + 1;
  const size_t height = bounds.max.y - bounds.min.y + 1;

  this->alpha = make_aligned_mdarray<uint8_t>(height, width);

  for (const auto &[x, y] : points_to_fill) {
    this->alpha[y - baseOffset.y, x - baseOffset.x] = ImgAlpha::FILL_VALUE;
  }

  for (ptrdiff_t i = 0; i < img.getHeight(); i++) {
    for (ptrdiff_t j = 0; j < img.getWidth(); j++) {
      if (img[i, j] == ImgAlpha::FILL_VALUE) {
        this->alpha[i - baseOffset.y, j - baseOffset.x] = ImgAlpha::FILL_VALUE;
      }
    }
  }
}

OffsettedImgAlpha::OffsettedImgAlpha(const ImgAlpha &img, const ptrdiff_t r) {
  const auto &contour = img.getContour();
  assert(!contour.empty());

  const auto disk = this->generateDisk(r);
  const auto [bounds, points_to_fill] = this->minkowskiSum(contour, disk, r);

  this->baseOffset = Vector{bounds.min.x, bounds.min.y};

  this->fill(bounds, points_to_fill, img);
}

OffsettedImgAlpha::~OffsettedImgAlpha() = default;
} // namespace gp