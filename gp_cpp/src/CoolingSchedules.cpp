#include "CoolingSchedules.hpp"

#include <cmath>

namespace gp {
double ExponentialSchedule::operator()(const double tInitial, const double t,
                                       const ptrdiff_t iteration) {
  static_cast<void>(t);
  return tInitial * std::pow(this->alpha, iteration);
}

double LinearSchedule::operator()(const double tInitial, const double t,
                                  const ptrdiff_t iteration) {
  static_cast<void>(tInitial);
  static_cast<void>(iteration);
  return this->k * t;
}
} // namespace gp