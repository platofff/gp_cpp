#pragma once

#include "BitImage.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>

namespace gp {
struct PlacementArea {
  Box bounds;
  Point canvasStart;
  Point imageStart;
};

class Canvas : public BitImage {
private:
  enum Area { BOTTOM, RIGHT, BOTTOM_RIGHT, CANVAS, AREAS_SIZE };

  const std::array<Box, AREAS_SIZE> areas;
  const std::array<Vector, AREAS_SIZE> offsets;
  using BitImage::data;

  std::vector<PlacementArea> placementAreas(const BitImage &img,
                                            const Point pos) const;

  void processImageIntersection(
      const BitImage &img, const Point pos,
      std::function<void(bool &, const bool &)> action) const;

  using nlp = std::numeric_limits<ptrdiff_t>;

public:
  Canvas(const ptrdiff_t width, const ptrdiff_t height);
  Canvas(Canvas &&other);

  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  void addImage(const BitImage &img, const Point pos);

  uint64_t intersectionArea(const BitImage &img, const Point pos) const;
};

} // namespace gp
