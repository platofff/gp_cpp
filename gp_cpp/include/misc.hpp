#pragma once

#include <functional>
#include <memory>
#include <new>
#include <mdspan>

namespace gp {
struct Point {
  ptrdiff_t x, y;
  bool operator==(const Point& other) const;
  inline Point translate(const Point& vec) const {
    return {this->x + vec.x, this->y + vec.y};
  }
};

using Vector = Point;

struct Box {
  Point min, max;
  bool valid = true;

  Box intersect(const Box& other) const {
    Box result = {{std::max(this->min.x, other.min.x),
                   std::max(this->min.y, other.min.y)},
                  {std::min(this->max.x, other.max.x),
                   std::min(this->max.y, other.max.y)}};

    if (result.min.x >= result.max.x || result.min.y >= result.max.y) {
      result.valid = false;
    }

    return result;
  }

  inline Box translate(const Vector& vec) const {
    return Box{{this->min.x + vec.x, this->min.y + vec.y},
               {this->max.x + vec.x, this->max.y + vec.y}};
  }

  inline ptrdiff_t getWidth() const { return this->max.x - this->min.x; }

  inline ptrdiff_t getHeight() const { return this->max.y - this->min.y; }
};

#if __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

template <typename T, size_t alignment>
struct AlignedArrayDeleter {
  void operator()(T* p) const {
    operator delete[](p, std::align_val_t(alignment));
  }
};

template <typename T, size_t alignment = hardware_destructive_interference_size>
using aligned_unique_array_ptr =
    std::unique_ptr<T[], AlignedArrayDeleter<T, alignment>>;

template <typename T, size_t alignment = hardware_destructive_interference_size>
aligned_unique_array_ptr<T, alignment> make_aligned_unique_array(size_t size) {
  T* ptr = new (std::align_val_t(alignment)) T[size]{};
  return aligned_unique_array_ptr<T, alignment>(ptr);
}

template <typename T, size_t dim>
class aligned_mdarray : public std::mdspan<T, std::dextents<size_t, dim>> {
 private:
  aligned_unique_array_ptr<T> buf{nullptr};
  size_t buf_size = 0;

  void move_from(aligned_mdarray&& other) noexcept {
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

  aligned_mdarray(const aligned_mdarray&) = delete;
  aligned_mdarray& operator=(const aligned_mdarray&) = delete;

  // Move constructor
  aligned_mdarray(aligned_mdarray&& other) noexcept {
    this->move_from(std::move(other));
  }

  // Move assignment operator
  aligned_mdarray& operator=(aligned_mdarray&& other) noexcept {
    if (this == &other) {
      return *this;  // self-assignment check
    }
    this->move_from(std::move(other));
    return *this;
  }

  inline size_t size() { return this->buf_size; }
};

template <typename T, typename... Extents>
auto make_aligned_mdarray(Extents... extents) {
  return aligned_mdarray<T, sizeof...(Extents)>{
      std::array<size_t, sizeof...(Extents)>{extents...}};
}

template <typename T>
T positive_modulo(const T i, const T n) {
  return (i % n + n) % n;
}

} // namespace gp

template <>
struct std::hash<gp::Point> {
  size_t operator()(const gp::Point& p) const {
    return hash<ptrdiff_t>()(p.x) ^ (hash<ptrdiff_t>()(p.y) << 1);
  }
};
