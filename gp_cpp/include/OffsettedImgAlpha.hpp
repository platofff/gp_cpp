#pragma once

#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"
#include <cstddef>

namespace gp {
class OffsettedImgAlpha : public ImgAlpha {
private:
  Vector baseOffset;
  static const aligned_mdarray<uint8_t, 2> generateDisk(const ptrdiff_t r);
  void minkowskiSum(const ImgAlphaFilledContour &img,
                    const aligned_mdarray<uint8_t, 2> &disk, const ptrdiff_t r);

protected:
  using ImgAlpha::alpha;

public:
  OffsettedImgAlpha(const ImgAlphaFilledContour &img, const ptrdiff_t r);
  Vector getBaseOffset() const;
};
} // namespace gp
