#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>

#include <boost/dynamic_bitset.hpp>

#include "ImgAlpha.hpp"
#include "boost/dynamic_bitset/dynamic_bitset.hpp"

namespace gp {
class BitImage {
private:
  ptrdiff_t height, width;

protected:
  boost::dynamic_bitset<> data;
  inline void setPixel(const size_t i, const size_t j, const bool value) {
    this->data[i * this->getWidth() + j] = value;
  }

public:
  BitImage(const ImgAlpha &img);
  BitImage(const size_t height, const size_t width);
  BitImage(BitImage &&other) noexcept;

  BitImage(const BitImage &) = delete;
  BitImage &operator=(const BitImage &) = delete;

  inline ptrdiff_t getWidth() const { return this->width; }
  inline ptrdiff_t getHeight() const { return this->height; }

  uint64_t nPixels() const;

  inline bool operator[](const size_t i, const size_t j) const {
    return data[i * this->getWidth() + j];
  }
};

std::ostream &operator<<(std::ostream &stream, const BitImage &image);

} // namespace gp
