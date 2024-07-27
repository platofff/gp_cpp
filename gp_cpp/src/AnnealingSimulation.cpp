#include "AnnealingSimulation.hpp"

#include <numbers>
#include <random>

namespace gp {
std::optional<Point> annealingSimulationOptimimizePlacement(
    const Canvas &canvas, const BitImage &img, const double temperatureInitial,
    const double lambda) {
  std::random_device rd;
  std::mt19937 randomGen(rd());

  std::uniform_int_distribution<ptrdiff_t> distX(0, canvas.getWidth() - 1);
  std::uniform_int_distribution<ptrdiff_t> distY(0, canvas.getHeight() - 1);

  Point currentPoint(distX(randomGen),
                     distY(randomGen)); // Start with a random position

  double currentCost = canvas.intersectionArea(img, currentPoint);
  const double rInitial = std::sqrt(canvas.getWidth() * canvas.getWidth() +
                                    canvas.getHeight() * canvas.getHeight());
  double rMax = rInitial;
  std::uniform_real_distribution<double> thetaDist(-std::numbers::pi_v<double>,
                                                   std::numbers::pi_v<double>);
  std::uniform_real_distribution<double> dist(0.0, 1.0);

  uint64_t i = 0;
  for (double temperature = temperatureInitial; temperature > 1e-3;
       temperature = temperature * std::exp(-lambda * i), i++) {
    std::uniform_real_distribution<double> rDist(0.0, rMax);
    double r = rDist(randomGen);
    double theta = thetaDist(randomGen);

    ptrdiff_t dx = r * std::cos(theta);
    ptrdiff_t dy = r * std::sin(theta);

    const auto canvasWidth = static_cast<ptrdiff_t>(canvas.getWidth());
    const auto canvasHeight = static_cast<ptrdiff_t>(canvas.getHeight());

    const Point nextPoint{
        (((currentPoint.getX() + dx) % canvasWidth) + canvasWidth) %
            canvasWidth,
        (((currentPoint.getY() + dy) % canvasHeight) + canvasHeight) %
            canvasHeight}; // Wrap coordinates

    double nextCost = canvas.intersectionArea(img, nextPoint);

    if (nextCost == 0) {
      return nextPoint; // Found an optimal placement with zero intersection
    }

    double costDifference = nextCost - currentCost;
    double acceptanceProbability = std::exp(-costDifference / temperature);

    if (costDifference < 0 || dist(randomGen) < acceptanceProbability) {
      currentPoint = nextPoint;
      currentCost = nextCost;
    }

    rMax = rInitial * std::exp(-lambda * i); // Reduce rMax
  }

  return std::nullopt; // Return nullopt if no placement found with zero
                       // intersection
}

} // namespace gp