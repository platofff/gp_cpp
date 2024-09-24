#pragma once

#include "ImgAlpha.hpp"

namespace gp {
class ImgAlphaFilledContour : public ImgAlpha {
private:
  enum class PixelState { NOT_CHECKED = 0, FILLED };

  template <typename Container>
  Container getFilteredPerimeter(std::function<bool(uint8_t)> predicate =
                                     [](uint8_t) { return true; }) {
    Container container;
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

    return container;
  }

  void generateAndFillContour(const uint8_t threshold);

public:
  using ImgAlpha::ImgAlpha;
  ImgAlphaFilledContour(const uint8_t *data, const size_t width,
                        const size_t height) = delete;
  ImgAlphaFilledContour(const uint8_t *data, const size_t width,
                        const size_t height, const uint8_t threshold);
  ImgAlphaFilledContour(ImgAlphaFilledContour &&other) noexcept;
};
} // namespace gp