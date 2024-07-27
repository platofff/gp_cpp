#include "misc.hpp"

namespace gp {
Point::Point(const ptrdiff_t x, const ptrdiff_t y) : x(x), y(y) {}

bool Point::operator==(const Point &other) const {
  return this->getX() == other.getX() && this->getY() == other.getY();
}

Point Point::translate(const Vector &vec) const {
  return {this->getX() + vec.getX(), this->getY() + vec.getY()};
}

ptrdiff_t Point::getX() const { return this->x; }

ptrdiff_t Point::getY() const { return this->y; }

std::ostream &operator<<(std::ostream &stream, const Point &point) {
  return stream << point.getX() << ";" << point.getY();
}

Box::Box(const Point &min, const Point &max) : min(min), max(max) {}

bool Box::isValid() const {
  if (!this->valid.has_value()) {
    this->valid = this->min.getX() <= this->max.getX() &&
                  this->min.getY() <= this->max.getY();
  }
  return this->valid.value();
}

Box Box::intersect(const Box &other) const {
  return Box{{std::max(this->min.getX(), other.min.getX()),
              std::max(this->min.getY(), other.min.getY())},
             {std::min(this->max.getX(), other.max.getX()),
              std::min(this->max.getY(), other.max.getY())}};
}

Box Box::translate(const Vector &vec) const {
  return Box{{this->min.getX() + vec.getX(), this->min.getY() + vec.getY()},
             {this->max.getX() + vec.getX(), this->max.getY() + vec.getY()}};
}

ptrdiff_t Box::getWidth() const {
  return this->max.getX() - this->min.getX() + 1;
}

ptrdiff_t Box::getHeight() const {
  return this->max.getY() - this->min.getY() + 1;
}

const Point &Box::getMin() const { return this->min; }

const Point &Box::getMax() const { return this->max; }

std::ostream &operator<<(std::ostream &stream, const Box &box) {
  return stream << "Box{" << box.getMin() << " " << box.getMax() << "}";
}
} // namespace gp