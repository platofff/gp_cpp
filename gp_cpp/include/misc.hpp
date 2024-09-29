#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mdspan>
#include <memory>
#include <new>

namespace gp {
template <typename T> class TPoint {
private:
  T x, y;

public:
  inline TPoint(const T x, const T y) : x(x), y(y){};

  inline bool operator==(const TPoint &other) const {
    return this->getX() == other.getX() && this->getY() == other.getY();
  };
  inline TPoint translate(const TPoint &vec) const {
    return {this->getX() + vec.getX(), this->getY() + vec.getY()};
  };

  inline T getX() const { return this->x; };
  inline T getY() const { return this->y; };

  friend std::ostream &operator<<(std::ostream &stream, const TPoint &point) {
    return stream << point.getX() << ";" << point.getY();
  }
};

using Point = TPoint<ptrdiff_t>;
using Vector = Point;

std::ostream &operator<<(std::ostream &stream, const Point &point);

class Box {
private:
  Point min, max;
  mutable std::optional<bool> valid;

public:
  inline Box(const Point &min, const Point &max) : min(min), max(max) {}
  inline bool isValid() const {
    if (!this->valid.has_value()) {
      this->valid = this->min.getX() <= this->max.getX() &&
                    this->min.getY() <= this->max.getY();
    }
    return *this->valid;
  }

  inline Box intersect(const Box &other) const {
    return Box{{std::max(this->min.getX(), other.min.getX()),
                std::max(this->min.getY(), other.min.getY())},
               {std::min(this->max.getX(), other.max.getX()),
                std::min(this->max.getY(), other.max.getY())}};
  }

  inline Box translate(const Vector &vec) const {
    return Box{{this->min.getX() + vec.getX(), this->min.getY() + vec.getY()},
               {this->max.getX() + vec.getX(), this->max.getY() + vec.getY()}};
  }

  inline ptrdiff_t getWidth() const {
    return this->max.getX() - this->min.getX() + 1;
  }

  inline ptrdiff_t getHeight() const {
    return this->max.getY() - this->min.getY() + 1;
  }

  inline const Point &getMin() const { return this->min; }

  inline const Point &getMax() const { return this->max; }
};

std::ostream &operator<<(std::ostream &stream, const Box &box);

#if __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

template <typename T, size_t alignment> struct AlignedArrayDeleter {
  void operator()(T *p) const {
    operator delete[](p, std::align_val_t(alignment));
  }
};

template <typename T, size_t alignment = hardware_destructive_interference_size>
using aligned_unique_array_ptr =
    std::unique_ptr<T[], AlignedArrayDeleter<T, alignment>>;

template <typename T, size_t alignment = hardware_destructive_interference_size>
aligned_unique_array_ptr<T, alignment> make_aligned_unique_array(size_t size) {
  T *ptr = new (std::align_val_t(alignment)) T[size]{};
  return aligned_unique_array_ptr<T, alignment>(ptr);
}

template <typename T, size_t dim>
class aligned_mdarray : public std::mdspan<T, std::dextents<size_t, dim>> {
private:
  aligned_unique_array_ptr<T> buf{nullptr};
  size_t buf_size;

  inline void move_from(aligned_mdarray &&other) noexcept {
    this->buf_size = other.buf_size;
    const auto extents = other.extents();
    this->buf = std::move(other.buf);
    std::mdspan<T, std::dextents<size_t, dim>>::operator=(
        {this->buf.get(), extents});
  }

public:
  inline aligned_mdarray(std::array<size_t, dim> extents) {
    this->buf_size = 1;
    for (size_t e : extents) {
      this->buf_size *= e;
    }

    this->buf = make_aligned_unique_array<T>(this->buf_size);
    std::mdspan<T, std::dextents<size_t, dim>>::operator=({buf.get(), extents});
  }

  inline aligned_mdarray() {}

  aligned_mdarray(const aligned_mdarray &) = delete;
  aligned_mdarray &operator=(const aligned_mdarray &) = delete;

  // Move constructor
  inline aligned_mdarray(aligned_mdarray &&other) noexcept {
    this->move_from(std::move(other));
  }

  // Move assignment operator
  inline aligned_mdarray &operator=(aligned_mdarray &&other) noexcept {
    if (this == &other) {
      return *this; // self-assignment check
    }
    this->move_from(std::move(other));
    return *this;
  }

  inline size_t size() { return this->buf_size; }
};

template <typename T, typename... Extents>
inline auto make_aligned_mdarray(Extents... extents) {
  return aligned_mdarray<T, sizeof...(Extents)>{
      std::array<size_t, sizeof...(Extents)>{extents...}};
}

template <typename T> inline T positive_modulo(const T i, const T n) {
  return (i % n + n) % n;
}

const aligned_mdarray<bool, 2> generateDisk(const ptrdiff_t r);

} // namespace gp

template <> struct std::hash<gp::Point> {
  size_t operator()(const gp::Point &p) const {
    return hash<ptrdiff_t>()(p.getX()) ^ (hash<ptrdiff_t>()(p.getY()) << 1);
  }
};
