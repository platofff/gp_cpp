#pragma once

#include <cstddef>
#include <optional>
#include <functional>

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "misc.hpp"

namespace gp {

std::optional<Point> optimizePlacement(
    const Canvas &canvas, const BitImage &img, const double tInitial,
    std::function<double(const double, const double, const ptrdiff_t)> decreaseT =
        [](const double tInitial, const double t, const ptrdiff_t iteration) {
          const double alpha = 0.9;
          return tInitial * std::pow(alpha, iteration);
        },
    const double eps = 0.0001);

} // namespace gp
