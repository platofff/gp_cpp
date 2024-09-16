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

  uint8_t &operator[](const size_t i, const size_t j) const;
  size_t getWidth() const;
  size_t getHeight() const;

  static constexpr uint8_t FILL_VALUE = 255;
};
} // namespace gp
