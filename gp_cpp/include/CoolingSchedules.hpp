#pragma once

#include <cmath>
#include <cstddef>

namespace gp {

struct ExponentialSchedule {
  double alpha;
  inline double operator()(const double tInitial, const double t,
                           const ptrdiff_t iteration) {
    static_cast<void>(t);
    return tInitial * std::pow(this->alpha, iteration);
  }
};

struct LinearSchedule {
  double k;
  inline double operator()(const double tInitial, const double t,
                           const ptrdiff_t iteration) {
    static_cast<void>(tInitial);
    static_cast<void>(iteration);
    return this->k * t;
  }
};

} // namespace gp