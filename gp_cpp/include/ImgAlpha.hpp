#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "misc.hpp"

namespace gp {
class ImgAlpha {
private:
  std::vector<Point> contour;

  enum class PixelState { NOT_CHECKED = 0, FILLED, CONTOUR };

  template <typename Container>
  void getFilteredPerimeter(
      Container &container,
      std::function<bool(uint8_t)> predicate = [](uint8_t) { return true; }) {
    ptrdiff_t i = 0, j = 0;

    // Top edge
    for (; i < this->getHeight(); i++) {
      if (predicate((*this)[i, j])) {
        container.emplace_back(j, i);
      }
    }
    i--;

    // Right edge
    for (; j < this->getWidth(); j++) {
      if (predicate((*this)[i, j])) {
        container.emplace_back(j, i);
      }
    }
    j--;

    // Bottom edge
    for (; i >= 0; i--) {
      if (predicate((*this)[i, j])) {
        container.emplace_back(j, i);
      }
    }
    i++;

    // Left edge
    for (; j > 0; j--) {
      if (predicate((*this)[i, j])) {
        container.emplace_back(j, i);
      }
    }
  }

protected:
  aligned_mdarray<uint8_t, 2> alpha;
  ImgAlpha();

public:
  ImgAlpha(const uint8_t *data, const size_t width, const size_t height);
  ImgAlpha(ImgAlpha &&other) noexcept;

  ImgAlpha(const ImgAlpha &other) = delete;
  ImgAlpha &operator=(const ImgAlpha &) = delete;

  ~ImgAlpha();

  uint8_t &operator[](size_t i, size_t j) const { return this->alpha[i, j]; }
  size_t getWidth() const { return this->alpha.extent(1); }
  size_t getHeight() const { return this->alpha.extent(0); }

  void generateAndFillContour(uint8_t threshold);
  const std::vector<Point> &getContour() const;

  static constexpr uint8_t FILL_VALUE = 255;
};
} // namespace gp
