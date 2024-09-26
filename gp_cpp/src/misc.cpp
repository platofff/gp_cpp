#include "misc.hpp"

namespace gp {
Box::Box(const Point &min, const Point &max) : min(min), max(max) {}

bool Box::isValid() const {
  if (!this->valid.has_value()) {
    this->valid = this->min.getX() <= this->max.getX() &&
                  this->min.getY() <= this->max.getY();
  }
  return *this->valid;
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

const aligned_mdarray<bool, 2> generateDisk(const ptrdiff_t r) {
  const size_t n = 2 * r + 1;
  auto disk = make_aligned_mdarray<bool>(n, n);

  for (ptrdiff_t i = 0; i < n; i++) {
    for (ptrdiff_t j = 0; j < n; j++) {
      const auto x = static_cast<double>(j - r);
      const auto y = static_cast<double>(i - r);
      const auto d = std::sqrt(x * x + y * y);
      if (d <= static_cast<double>(r)) {
        disk[i, j] = true;
      }
    }
  }

  return disk;
}

} // namespace gp