#include "BitImage.hpp"
#include <cstddef>

namespace gp {
BitImage::BitImage(const ImgAlpha &img)
    : data(make_aligned_mdarray<bool>(img.getHeight(), img.getWidth())) {
  for (size_t row_idx = 0; row_idx < img.getHeight(); row_idx++) {
    for (size_t col_idx = 0; col_idx < img.getWidth(); col_idx++) {
      this->data[row_idx, col_idx] =
          img[row_idx, col_idx] == ImgAlpha::FILL_VALUE;
    }
  }
}

BitImage::BitImage(BitImage &&other) noexcept : data(std::move(other.data)) {}

ptrdiff_t BitImage::getWidth() const { return this->data.extent(1); }
ptrdiff_t BitImage::getHeight() const { return this->data.extent(0); }

uint64_t BitImage::nPixels() const {
  uint64_t res = 0;
  for (size_t i = 0; i < this->getHeight(); i++) {
    for (size_t j = 0; j < this->getWidth(); j++) {
      if (this->data[i, j]) {
        res++;
      }
    }
  }
  return res;
}

const bool &BitImage::operator[](const size_t i, const size_t j) const {
  return this->data[i, j];
}

std::ostream &operator<<(std::ostream &stream, const BitImage &image) {
  for (ptrdiff_t i = 0; i < image.getHeight(); i++) {
    for (ptrdiff_t j = 0; j < image.getWidth(); j++) {
      stream << image[i, j];
    }
    stream << std::endl;
  }

  return stream;
}
} // namespace gp
