#pragma once

#include <cstddef>
#include <optional>

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "misc.hpp"

namespace gp {

std::optional<Point> optimizePlacement(
    const Canvas &canvas, const BitImage &img, const double tInitial);

} // namespace gp
