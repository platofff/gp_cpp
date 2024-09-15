#include "OptimizePlacement.hpp"
#include "misc.hpp"

#include <cstddef>
#include <optional>
#include <random>

namespace gp {

double decreaseTemperature(const double t) {
  const double gamma = 0.99; // TODO
  return gamma * t;
}

std::optional<Point> optimizePlacement(const Canvas &canvas,
                                       const BitImage &img,
                                       const double tInitial) {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<double> probDist(0.0, 1.0);

  const auto posToWrappedPoint = [&canvas](const ptrdiff_t x,
                                           const ptrdiff_t y) {
    return Point{
        ((x % canvas.getWidth()) + canvas.getWidth()) % canvas.getWidth(),
        ((y % canvas.getHeight()) + canvas.getHeight()) % canvas.getHeight()};
  };

  double t = tInitial;
  double eps = 0.0001;
  // random point on the canvas
  Point currentPosition{
      std::uniform_int_distribution<ptrdiff_t>(0, canvas.getWidth() - 1)(dev),
      std::uniform_int_distribution<ptrdiff_t>(0, canvas.getHeight() - 1)(dev)};
  ptrdiff_t currentResult = canvas.intersectionArea(img, currentPosition);
  if (currentResult == 0) {
    return currentPosition;
  }

  using FPVector = TPoint<double>;
  const FPVector deltaMaxInitial{
      std::ceil(static_cast<double>(canvas.getWidth()) / 2.0),
      std::ceil(static_cast<double>(canvas.getHeight()) / 2.0)};

  for (; t > eps; t = decreaseTemperature(t)) {
    const Vector deltaMax{
        static_cast<ptrdiff_t>(deltaMaxInitial.getX() * (t / tInitial)),
        static_cast<ptrdiff_t>(deltaMaxInitial.getY() * (t / tInitial))};
    std::uniform_int_distribution<ptrdiff_t> distX(-deltaMax.getX(),
                                                   deltaMax.getX());
    std::uniform_int_distribution<ptrdiff_t> distY(-deltaMax.getY(),
                                                   deltaMax.getY());
    const Vector delta{distX(dev), distY(dev)};

    const Point newPosition =
        posToWrappedPoint(currentPosition.getX() + delta.getX(),
                          currentPosition.getY() + delta.getY());
    const ptrdiff_t newResult = canvas.intersectionArea(img, newPosition);

    const ptrdiff_t deltaResult = currentResult - newResult;
    if (deltaResult > 0 // new result is better
        || std::exp(-(static_cast<double>(deltaResult) / t)) <
               probDist(dev) // accepting worse solution
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

} // namespace gp