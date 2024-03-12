#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <memory>

#include <iostream>

#include "ImgAlpha.hpp"

namespace gp {
template <typename T, bool GenOffsetViews = true> class BitImage {
private:
  size_t width;

public:
  static constexpr ptrdiff_t BIT_SIZE = sizeof(T) * 8;
  static constexpr size_t N_VIEWS = GenOffsetViews ? BIT_SIZE : 1;

  BitImage(const ImgAlpha &img) {
    size_t w = 0;

    if constexpr (GenOffsetViews) {
      // max width including BIT_SIZE - 1 bit shift
      w = img.getWidth() + BIT_SIZE - 1;
    } else {
      w = img.getWidth();
    }
    this->width = img.getWidth();

    const auto t_width = w / BIT_SIZE + (w % BIT_SIZE == 0 ? 0 : 1);
    const auto buf_size = t_width * img.getHeight();

    for (size_t i = 0; i < N_VIEWS; i++) {
      this->data[i] = make_aligned_mdarray<T>(img.getHeight(), t_width);
    }

    for (size_t row_idx = 0; row_idx < img.getHeight(); row_idx++) {
      for (size_t col_idx = 0; col_idx < img.getWidth(); col_idx++) {
        if (img[row_idx, col_idx] != ImgAlpha::FILL_VALUE) {
          continue;
        }

        const T lsb = 1;

        for (size_t buf_idx = 0; buf_idx < N_VIEWS; buf_idx++) {
          const auto global_bit_idx = col_idx + buf_idx;

          const auto byte_idx = global_bit_idx / BIT_SIZE;
          const auto bit_idx = global_bit_idx % BIT_SIZE;

          const auto &view = this->data[buf_idx];
          view[row_idx, byte_idx] |= lsb << bit_idx;
        }
      }
    }
  }

  BitImage(const BitImage &) = delete;
  BitImage &operator=(const BitImage &) = delete;

  BitImage(BitImage &&other) : width(other.width), data(std::move(other.data)) {}

  ~BitImage() = default;

  inline ptrdiff_t getWidth() const { return this->data[0].extent(1); }
  inline ptrdiff_t getHeight() const { return this->data[0].extent(0); }

  inline ptrdiff_t getBitWidth() const { return this->width; }
  inline size_t getViewsNumber() const { return this->n_views; }

  inline auto &getOffsettedView(size_t offset) const {
    assert(offset < N_VIEWS);
    return data[offset];
  }

  template <std::size_t N> static void reverse(std::bitset<N> &b) {
    for (std::size_t i = 0; i < N / 2; ++i) {
      bool t = b[i];
      b[i] = b[N - i - 1];
      b[N - i - 1] = t;
    }
  }

  void debug() {
    for (auto idx = 0; idx < N_VIEWS; idx++) {
      std::cout << "Image view " << idx << ":" << std::endl;
      const auto &view = this->getOffsettedView(idx);
      for (auto i = 0; i < view.extent(0); i++) {
        for (auto j = 0; j < view.extent(1); j++) {
          auto bs = static_cast<std::bitset<BIT_SIZE>>(this->data[idx][i, j]);
          reverse<BIT_SIZE>(bs);
          std::cout << bs << " ";
        }
        std::cout << std::endl;
      }
    }
  }

  int nPixels() const {
    int res = 0;
    for (auto i = 0; i < this->getHeight(); i++) {
      for (auto j = 0; j < this->getWidth(); j++) {
        res += std::popcount(this->data[0][i, j]);
      }
    }
    return res;
  }

protected:
  std::array<aligned_mdarray<T, 2>, N_VIEWS> data;
};
} // namespace gp
