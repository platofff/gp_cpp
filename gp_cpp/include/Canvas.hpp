#pragma once

#include "BitImage.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>
#include <random>

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

  using FPVector = TPoint<double>;
  const FPVector deltaMaxInitial;

  std::mt19937 &rng;

  std::vector<PlacementArea> placementAreas(const BitImage &img,
                                            const Point pos) const;

  void processImageIntersection(
      const BitImage &img, const Point pos,
      std::function<void(bool &, const bool &)> action) const;

  using nlp = std::numeric_limits<ptrdiff_t>;

  Point wrapPosition(const ptrdiff_t x, const ptrdiff_t y) const;
  uint64_t intersectionArea(const BitImage &img, const Point pos) const;

public:
  Canvas(const ptrdiff_t width, const ptrdiff_t height, std::mt19937 &rng);
  Canvas(Canvas &&other);

  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  std::optional<Point> optimizePlacement(
      const BitImage &img, const double tInitial,
      std::function<double(const double, const double, const ptrdiff_t)>
          decreaseT =
              [](const double tInitial, const double t,
                 const ptrdiff_t iteration) {
                const double alpha = 0.9;
                return tInitial * std::pow(alpha, iteration);
              },
      const double eps = 0.0001) const;

  void addImage(const BitImage &img, const Point pos);
};

} // namespace gp
