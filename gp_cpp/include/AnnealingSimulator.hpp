#pragma once

#include <cstddef>
#include <random>

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "misc.hpp"

namespace gp {
template <typename T> class AnnealingSimulator {
private:
  const double alpha;
  const double temperature;
  std::random_device rd;
  std::mt19937 random_gen;

public:
  AnnealingSimulator(double alpha, double temperature)
      : alpha(alpha), temperature(temperature) {
    this->random_gen = std::mt19937(this->rd());
  };

  ~AnnealingSimulator() = default;

  std::optional<Point> optimimizePlacement(const Canvas<T> &canvas,
                                           const BitImage<T> &img) {
    double alpha = this->alpha;
    double temperature = this->temperature;
    std::uniform_int_distribution<int> distX(0, canvas.getBitWidth() - 1);
    std::uniform_int_distribution<int> distY(0, canvas.getHeight() - 1);

    Point currentPoint(distX(this->random_gen),
                       distY(this->random_gen)); // Start with a random position

    double currentCost = canvas.intersectionArea(img, currentPoint);

    while (temperature > 1e-3) {       // Threshold for temperature
      int dx = (this->random_gen() % 3) - 1; // -1, 0, 1
      int dy = (this->random_gen() % 3) - 1; // -1, 0, 1

      Point nextPoint(currentPoint.x + dx, currentPoint.y + dy);

      // Make sure the nextPoint is within the canvas
      nextPoint.x = std::clamp(nextPoint.x, static_cast<ptrdiff_t>(0), canvas.getBitWidth() - 1);
      nextPoint.y = std::clamp(nextPoint.y, static_cast<ptrdiff_t>(0), canvas.getHeight() - 1);

      double nextCost = canvas.intersectionArea(img, nextPoint);

      if (nextCost == 0) {
        return nextPoint; // Found an optimal placement with zero intersection
      }

      double costDifference = nextCost - currentCost;
      double acceptanceProbability = std::exp(-costDifference / temperature);

      std::uniform_real_distribution<double> dist(0.0, 1.0);
      if (costDifference < 0 || dist(random_gen) < acceptanceProbability) {
        currentPoint = nextPoint;
        currentCost = nextCost;
      }

      temperature *= alpha; // Reduce the temperature
    }

    return std::nullopt; // Return nullopt if no placement found with zero
                         // intersection
  }
};
} // namespace gp
