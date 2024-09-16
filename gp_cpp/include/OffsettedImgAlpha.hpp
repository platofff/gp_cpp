#pragma once

#include "ImgAlpha.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "misc.hpp"
#include <cstddef>
#include <unordered_set>

namespace gp {
class OffsettedImgAlpha : public ImgAlpha {
private:
  Vector baseOffset;
  static const aligned_mdarray<uint8_t, 2> generateDisk(const ptrdiff_t r);
  static std::pair<const Box, const std::unordered_set<Point>>
  minkowskiSum(const std::vector<Point> &contour,
               const aligned_mdarray<uint8_t, 2> &disk, const ptrdiff_t r);
  void fill(const Box &bounds, const std::unordered_set<Point> &points_to_fill, const ImgAlpha &img);

protected:
  using ImgAlpha::alpha;

public:
  OffsettedImgAlpha(const ImgAlphaFilledContour &img, const ptrdiff_t r);
  Vector getBaseOffset() const;
};
} // namespace gp
