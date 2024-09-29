#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

#include "ImgAlpha.hpp"

namespace gp {
class BitImage {
public:
  using Block = uint64_t;
  static constexpr size_t bits_per_block = sizeof(Block) * 8;

private:
  ptrdiff_t height, width;
  size_t bitset_size;

protected:
  std::vector<uint64_t> data;
  inline void setPixel(const size_t i, const size_t j, const bool value) {
    const size_t index = i * this->getWidth() + j;
    const size_t vector_index =
        index /
        bits_per_block; // Calculate which uint64_t element contains the bit
    const size_t bit_index =
        index %
        bits_per_block; // Find the bit position within that uint64_t element
    if (value) {
      data[vector_index] |= (static_cast<Block>(1) << bit_index); // Set the bit
    } else {
      data[vector_index] &=
          ~(static_cast<Block>(1) << bit_index); // Clear the bit
    }
  }

public:
  inline BitImage(const ImgAlpha &img)
      : height(img.getHeight()), width(img.getWidth()),
        bitset_size(height * width) {
    size_t num_elements = (bitset_size + bits_per_block - 1) / bits_per_block;
    data.resize(num_elements, 0);
    for (size_t row_idx = 0; row_idx < img.getHeight(); row_idx++) {
      for (size_t col_idx = 0; col_idx < img.getWidth(); col_idx++) {
        this->setPixel(row_idx, col_idx,
                       img[row_idx, col_idx] == ImgAlpha::FILL_VALUE);
      }
    }
  }
  inline BitImage(const size_t height, const size_t width)
      : height(height), width(width), bitset_size(height * width) {
    size_t num_elements = (bitset_size + bits_per_block - 1) / bits_per_block;
    data.resize(num_elements, 0);
  }

  inline BitImage(BitImage &&other) noexcept
      : height(other.height), width(other.width),
        bitset_size(other.bitset_size), data(std::move(other.data)) {}

  BitImage(const BitImage &) = delete;
  BitImage &operator=(const BitImage &) = delete;

  inline ptrdiff_t getWidth() const { return this->width; }
  inline ptrdiff_t getHeight() const { return this->height; }

  uint64_t nPixels() const {
    uint64_t count = 0;
    for (const auto &elem : data) {
      count += std::popcount(elem); // Count set bits in each element
    }
    return count;
  }

  inline bool operator[](const size_t i, const size_t j) const {
    size_t index = i * this->getWidth() + j;
    size_t vector_index = index / bits_per_block;
    size_t bit_index = index % bits_per_block;
    return (data[vector_index] >> bit_index) & 1;
  }

  inline const std::vector<uint64_t> &getData() const { return this->data; }
};

std::ostream &operator<<(std::ostream &stream, const BitImage &image);

} // namespace gp
