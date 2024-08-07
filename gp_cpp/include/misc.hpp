#pragma once

#include <cstddef>
#include <functional>
#include <iostream>
#include <mdspan>
#include <memory>
#include <new>

namespace gp {
class Point {
private:
  ptrdiff_t x, y;

public:
  Point(const ptrdiff_t x, const ptrdiff_t y);

  bool operator==(const Point &other) const;
  Point translate(const Point &vec) const;

  ptrdiff_t getX() const;
  ptrdiff_t getY() const;
};

std::ostream &operator<<(std::ostream &stream, const Point &point);

using Vector = Point;

class Box {
private:
  Point min, max;
  mutable std::optional<bool> valid;

public:
  Box(const Point &min, const Point &max);

  Box intersect(const Box &other) const;
  Box translate(const Vector &vec) const;

  ptrdiff_t getWidth() const;
  ptrdiff_t getHeight() const;

  bool isValid() const;

  const Point &getMin() const;
  const Point &getMax() const;
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

  void move_from(aligned_mdarray &&other) noexcept {
    this->buf_size = other.buf_size;
    const auto extents = other.extents();
    this->buf = std::move(other.buf);
    std::mdspan<T, std::dextents<size_t, dim>>::operator=(
        {this->buf.get(), extents});
  }

public:
  aligned_mdarray(std::array<size_t, dim> extents) {
    this->buf_size = 1;
    for (size_t e : extents) {
      this->buf_size *= e;
    }

    this->buf = make_aligned_unique_array<T>(this->buf_size);
    std::mdspan<T, std::dextents<size_t, dim>>::operator=({buf.get(), extents});
  }

  aligned_mdarray() {}

  aligned_mdarray(const aligned_mdarray &) = delete;
  aligned_mdarray &operator=(const aligned_mdarray &) = delete;

  // Move constructor
  aligned_mdarray(aligned_mdarray &&other) noexcept {
    this->move_from(std::move(other));
  }

  // Move assignment operator
  aligned_mdarray &operator=(aligned_mdarray &&other) noexcept {
    if (this == &other) {
      return *this; // self-assignment check
    }
    this->move_from(std::move(other));
    return *this;
  }

  size_t size() { return this->buf_size; }
};

template <typename T, typename... Extents>
auto make_aligned_mdarray(Extents... extents) {
  return aligned_mdarray<T, sizeof...(Extents)>{
      std::array<size_t, sizeof...(Extents)>{extents...}};
}

template <typename T> T positive_modulo(const T i, const T n) {
  return (i % n + n) % n;
}

} // namespace gp

template <> struct std::hash<gp::Point> {
  size_t operator()(const gp::Point &p) const {
    return hash<ptrdiff_t>()(p.getX()) ^ (hash<ptrdiff_t>()(p.getY()) << 1);
  }
};
