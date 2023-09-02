#include "misc.hpp"

namespace gp {
  bool Point::operator==(const Point &other) const {
    return this->x == other.x && this->y == other.y;
  }
}