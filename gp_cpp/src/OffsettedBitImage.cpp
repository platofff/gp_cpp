#include "OffsettedBitImage.hpp"
#include "BitImage.hpp"
#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "OffsettedBitImage.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>

namespace gp {
OffsettedBitImage::OffsettedBitImage(const ImgAlphaFilledContour &img,
                                     const aligned_mdarray<bool, 2> &disk,
                                     const size_t r)
    : BitImage(img.getHeight() + 2 * r, img.getWidth() + 2 * r),
      baseOffset(0, 0) {
  this->baseOffset =
      Vector{-static_cast<ptrdiff_t>(r), -static_cast<ptrdiff_t>(r)};

  for (ptrdiff_t img_i = 0; img_i < img.getHeight(); img_i++) {
    for (ptrdiff_t img_j = 0; img_j < img.getWidth(); img_j++) {
      if (img[img_i, img_j] != img.FILL_VALUE) {
        continue;
      }
      const Point pos{img_j, img_i};

      for (ptrdiff_t i = 0; i < disk.extent(0); i++) {
        for (ptrdiff_t j = 0; j < disk.extent(1); j++) {
          if (!disk[i, j]) {
            continue;
          }
          const auto x = pos.getX() + j;
          const auto y = pos.getY() + i;
          this->setPixel(y, x, true);
        }
      }
    }
  }
}
} // namespace gp