#pragma once

#include "BitImage.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"
#include <cstddef>

namespace gp {
class OffsettedBitImage : public BitImage {
private:
  Vector baseOffset;

protected:
  using BitImage::data;

public:
  OffsettedBitImage(const ImgAlphaFilledContour &img,
                    const aligned_mdarray<bool, 2> &disk, const size_t r);
  const Vector &getBaseOffset() const;
};
} // namespace gp
