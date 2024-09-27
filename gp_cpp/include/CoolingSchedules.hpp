#pragma once

#include <cstddef>

namespace gp {

struct ExponentialSchedule {
  double alpha;
  double operator()(const double tInitial, const double t,
                    const ptrdiff_t iteration);
};

struct LinearSchedule {
  double k;
  double operator()(const double tInitial, const double t,
                    const ptrdiff_t iteration);
};

} // namespace gp