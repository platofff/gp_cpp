#pragma once

//#include <cassert>
//#include <iostream>

#include "ImgAlpha.hpp"

namespace gp {
class BitImage {
public:
  BitImage(const ImgAlpha &img) {
    size_t w = 0;
    this->data = make_aligned_mdarray<bool>(img.getHeight(), img.getWidth());

    for (size_t row_idx = 0; row_idx < img.getHeight(); row_idx++) {
      for (size_t col_idx = 0; col_idx < img.getWidth(); col_idx++) {
        this->data[row_idx, col_idx] = img[row_idx, col_idx] == ImgAlpha::FILL_VALUE;
      }
    }
  }

  BitImage(const BitImage &) = delete;
  BitImage &operator=(const BitImage &) = delete;

  BitImage(BitImage &&other) : data(std::move(other.data)) {}

  ~BitImage() = default;

  inline ptrdiff_t getWidth() const { return this->data.extent(1); }
  inline ptrdiff_t getHeight() const { return this->data.extent(0); }

  /*
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
  }*/

  int nPixels() const {
    int res = 0;
    for (auto i = 0; i < this->getHeight(); i++) {
      for (auto j = 0; j < this->getWidth(); j++) {
        if (this->data[i, j]) {
          res++;
        }
      }
    }
    return res;
  }

  aligned_mdarray<bool, 2> data;
};
} // namespace gp
