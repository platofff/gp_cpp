#pragma once

#include <cstddef>

namespace gp {

struct ExponentialSchedule {
  const double alpha;
  double operator()(const double tInitial, const double t,
                    const ptrdiff_t iteration);
};

struct LinearSchedule {
  const double k;
  double operator()(const double tInitial, const double t,
                    const ptrdiff_t iteration);
};

} // namespace gp