#pragma once

#include "ImgAlpha.hpp"
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
  OffsettedImgAlpha(const ImgAlpha &img, const ptrdiff_t r);
  ~OffsettedImgAlpha();
  inline Vector getBaseOffset() const {
    return this->baseOffset;
  }
};
} // namespace gp
