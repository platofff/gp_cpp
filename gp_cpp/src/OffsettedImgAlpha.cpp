#include "OffsettedImgAlpha.hpp"
#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

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

void OffsettedImgAlpha::minkowskiSum(const ImgAlphaFilledContour &img,
                                     const aligned_mdarray<uint8_t, 2> &disk,
                                     const ptrdiff_t r) {
  using nlp = std::numeric_limits<ptrdiff_t>;

  const auto height = img.getHeight() + 2 * r;
  const auto width = img.getWidth() + 2 * r;

  this->alpha = make_aligned_mdarray<uint8_t>(height, width);
  this->baseOffset = Vector{-r, -r};

  for (ptrdiff_t img_i = 0; img_i < img.getHeight(); img_i++) {
    for (ptrdiff_t img_j = 0; img_j < img.getWidth(); img_j++) {
      if (img[img_i, img_j] != img.FILL_VALUE) {
        continue;
      }
      const Point pos{img_j, img_i};

      for (ptrdiff_t i = 0; i < disk.extent(0); i++) {
        for (ptrdiff_t j = 0; j < disk.extent(1); j++) {
          if (disk[i, j] != ImgAlpha::FILL_VALUE) {
            continue;
          }
          const auto x = pos.getX() + j;
          const auto y = pos.getY() + i;
          this->alpha[y, x] = ImgAlpha::FILL_VALUE;
        }
      }
    }
  }
}

OffsettedImgAlpha::OffsettedImgAlpha(const ImgAlphaFilledContour &img,
                                     const ptrdiff_t r)
    : baseOffset(0, 0) {
  //std::cout << img << std::endl;
  const auto disk = this->generateDisk(r);
  this->minkowskiSum(img, disk, r);
  //std::cout << *this << std::endl;
}

Vector OffsettedImgAlpha::getBaseOffset() const { return this->baseOffset; }
} // namespace gp