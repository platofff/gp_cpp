#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "misc.hpp"

namespace gp {
class ImgAlpha {
protected:
  aligned_mdarray<uint8_t, 2> alpha;
  ImgAlpha() = default;

public:
  ImgAlpha(const uint8_t *data, const size_t width, const size_t height);
  ImgAlpha(ImgAlpha &&other) noexcept;

  ImgAlpha(const ImgAlpha &other) = delete;
  ImgAlpha &operator=(const ImgAlpha &) = delete;

  ~ImgAlpha() = default;

  inline uint8_t &operator[](const size_t i, const size_t j) const {
    return this->alpha[i, j];
  }

  inline size_t getWidth() const { return this->alpha.extent(1); }
  inline size_t getHeight() const { return this->alpha.extent(0); }

  static constexpr uint8_t FILL_VALUE = 255;
};

std::ostream &operator<<(std::ostream &stream, const ImgAlpha &img);
} // namespace gp
