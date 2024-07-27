#pragma once

#include <cstddef>
#include <optional>

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "misc.hpp"

namespace gp {

std::optional<Point> annealingSimulationOptimimizePlacement(
    const Canvas &canvas, const BitImage &img, const double temperatureInitial,
    const double lambda = 0.005);

} // namespace gp
