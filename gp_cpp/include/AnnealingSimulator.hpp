#pragma once

#include <random>
#include <numbers>
#include <optional>

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "misc.hpp"

namespace gp {
class AnnealingSimulator {
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

  std::optional<Point> optimimizePlacement(const Canvas &canvas,
                                           const BitImage &img) {
    double lambda = 0.005;
    double alpha = this->alpha;
    std::uniform_int_distribution<int> distX(0, canvas.getWidth() - 1);
    std::uniform_int_distribution<int> distY(0, canvas.getHeight() - 1);

    Point currentPoint(distX(this->random_gen),
                       distY(this->random_gen)); // Start with a random position

    double currentCost = canvas.intersectionArea(img, currentPoint);
    const double r_initial =
        std::sqrt(canvas.getWidth() * canvas.getWidth() +
                  canvas.getHeight() * canvas.getHeight());
    double r_max = r_initial;
    std::uniform_real_distribution<double> theta_dist(-std::numbers::pi_v<double>,
                                                      std::numbers::pi_v<double>);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    uint64_t i = 0;
    for (double temperature = this->temperature; temperature > 1e-3;
         temperature = this->temperature * std::exp(-lambda * i),
                i++) {
      std::uniform_real_distribution<double> r_dist(0.0, r_max);
      double r = r_dist(random_gen);
      double theta = theta_dist(random_gen);

      ptrdiff_t dx = r * std::cos(theta);
      ptrdiff_t dy = r * std::sin(theta);

      const Point next_point{
          (((currentPoint.x + dx) % canvas.getWidth()) +
           canvas.getWidth()) %
              canvas.getWidth(),
          (((currentPoint.y + dy) % canvas.getHeight()) + canvas.getHeight()) %
              canvas.getHeight()}; // Wrap coordinates

      double next_cost = canvas.intersectionArea(img, next_point);

      if (next_cost == 0) {
        return next_point; // Found an optimal placement with zero intersection
      }

      double costDifference = next_cost - currentCost;
      double acceptanceProbability = std::exp(-costDifference / temperature);

      if (costDifference < 0 || dist(random_gen) < acceptanceProbability) {
        currentPoint = next_point;
        currentCost = next_cost;
      }

      r_max = r_initial * std::exp(-lambda * i);       // Reduce r_max
    }

    return std::nullopt; // Return nullopt if no placement found with zero
                         // intersection
  }
};
} // namespace gp
