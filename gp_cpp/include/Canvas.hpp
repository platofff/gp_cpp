#pragma once

#include "BitImage.hpp"
#include "misc.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
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
                                            const Point pos) const noexcept;

  using nlp = std::numeric_limits<ptrdiff_t>;

  Point wrapPosition(const ptrdiff_t x, const ptrdiff_t y) const noexcept;
  uint64_t intersectionArea(const BitImage &img,
                            const Point &pos) const noexcept;

public:
  Canvas(const ptrdiff_t width, const ptrdiff_t height, std::mt19937 &rng);
  Canvas(Canvas &&other);

  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  template <typename CoolingSchedule>
  std::optional<Point>
  optimizePlacement(const BitImage &img, const double tInitial,
                    CoolingSchedule decreaseT,
                    const double eps = 0.0001) const noexcept {
    std::uniform_real_distribution<double> probDist(0.0, 1.0);

    double t = tInitial;
    // random point on the canvas
    Point currentPosition{std::uniform_int_distribution<ptrdiff_t>(
                              0, this->getWidth() - 1)(this->rng),
                          std::uniform_int_distribution<ptrdiff_t>(
                              0, this->getHeight() - 1)(this->rng)};
    ptrdiff_t currentResult = this->intersectionArea(img, currentPosition);
    if (currentResult == 0) {
      return currentPosition;
    }

    for (ptrdiff_t i = 0; t > eps; i++) {
      t = decreaseT(tInitial, t, i);
      const Vector deltaMax{
          static_cast<ptrdiff_t>(this->deltaMaxInitial.getX() * (t / tInitial)),
          static_cast<ptrdiff_t>(this->deltaMaxInitial.getY() *
                                 (t / tInitial))};
      std::uniform_int_distribution<ptrdiff_t> distX(-deltaMax.getX(),
                                                     deltaMax.getX());
      std::uniform_int_distribution<ptrdiff_t> distY(-deltaMax.getY(),
                                                     deltaMax.getY());
      const Vector delta{distX(this->rng), distY(this->rng)};

      const Point newPosition =
          this->wrapPosition(currentPosition.getX() + delta.getX(),
                             currentPosition.getY() + delta.getY());
      const ptrdiff_t newResult = this->intersectionArea(img, newPosition);

      const ptrdiff_t deltaResult = currentResult - newResult;
      if (deltaResult > 0 // new result is better
          || std::exp(-(static_cast<double>(deltaResult) / t)) <
                 probDist(this->rng) // accepting worse solution
      ) {
        currentPosition = newPosition;
        currentResult = newResult;
        if (currentResult == 0) {
          return currentPosition;
        }
      }
    }

    return std::nullopt;
  }

  void addImage(const BitImage &img, const Point &pos) noexcept;
};

} // namespace gp
