#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>

#include "ImgAlpha.hpp"

namespace gp {
class BitImage {
protected:
  aligned_mdarray<bool, 2> data;

public:
  BitImage(const ImgAlpha &img);
  BitImage(BitImage &&other) noexcept;

  BitImage(const BitImage &) = delete;
  BitImage &operator=(const BitImage &) = delete;

  ptrdiff_t getWidth() const;
  ptrdiff_t getHeight() const;

  uint64_t nPixels() const;

  const bool &operator[](const size_t i, const size_t j) const;
};

std::ostream &operator<<(std::ostream &stream, const BitImage &image);

} // namespace gp
