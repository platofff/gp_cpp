#include "CoolingSchedules.hpp"

#include <cmath>

namespace gp {
double ExponentialCooling::operator()(const double tInitial, const double t,
                                      const ptrdiff_t iteration){
  return tInitial * std::pow(this->alpha, iteration);
}

double LinearCooling::operator()(const double tInitial, const double t,
                                 const ptrdiff_t iteration) {
  return this->k * t;
}
} // namespace gp