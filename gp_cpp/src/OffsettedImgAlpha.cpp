#include "OffsettedImgAlpha.hpp"
#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>
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
OffsettedImgAlpha::minkowskiSum(const ImgAlphaFilledContour &img,
                                const aligned_mdarray<uint8_t, 2> &disk,
                                const ptrdiff_t r) {
  using nlp = std::numeric_limits<ptrdiff_t>;
  ptrdiff_t min_x = nlp::max();
  ptrdiff_t min_y = nlp::max();
  ptrdiff_t max_x = nlp::min();
  ptrdiff_t max_y = nlp::min();
  std::unordered_set<Point> points_to_fill; // TODO: replace to bitmap

  for (ptrdiff_t img_i = 0; img_i < img.getHeight(); img_i++) {
    for (ptrdiff_t img_j = 0; img_j < img.getWidth(); img_j++) {
      if (img[img_i, img_j] != img.FILL_VALUE) {
        continue;
      }
      const Point pos{img_j - r, img_i - r};

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
  }

  const Box bounds{{min_x, min_y}, {max_x, max_y}};
  return std::make_pair(bounds, points_to_fill);
}

void OffsettedImgAlpha::fill(const Box &bounds,
                             const std::unordered_set<Point> &points_to_fill,
                             const ImgAlpha &img) {
  const size_t width = bounds.getWidth();
  const size_t height = bounds.getHeight();

  std::cout << bounds << std::endl;

  this->alpha = make_aligned_mdarray<uint8_t>(width, height);

  for (const auto &point : points_to_fill) {
    this->alpha[point.getY() - baseOffset.getY(),
                point.getX() - baseOffset.getX()] = ImgAlpha::FILL_VALUE;
  }

  /*
  for (ptrdiff_t i = 0; i < img.getHeight(); i++) {
    for (ptrdiff_t j = 0; j < img.getWidth(); j++) {
      if (img[i, j] == ImgAlpha::FILL_VALUE) {
        this->alpha[i - baseOffset.getY(), j - baseOffset.getX()] =
            ImgAlpha::FILL_VALUE;
      }
    }
  }*/
}

OffsettedImgAlpha::OffsettedImgAlpha(const ImgAlphaFilledContour &img,
                                     const ptrdiff_t r)
    : baseOffset(0, 0) {
  std::cout << img << std::endl;
  const auto disk = this->generateDisk(r);

/*
  for (ptrdiff_t i = 0; i < disk.extent(0); i++) {
    for (ptrdiff_t j = 0; j < disk.extent(1); j++) {
      std::cout << static_cast<int>(disk[i, j] == 255);
    }
    std::cout << std::endl;
  }*/

  const auto [bounds, points_to_fill] = this->minkowskiSum(img, disk, r);

  this->baseOffset = bounds.getMin();

  this->fill(bounds, points_to_fill, img);
  std::cout << *this << std::endl;
}

Vector OffsettedImgAlpha::getBaseOffset() const { return this->baseOffset; }
} // namespace gp