#include "BitImage.hpp"
#include <cstddef>

namespace gp {
BitImage::BitImage(const ImgAlpha &img)
    : height(img.getHeight()),
      width(img.getWidth()), data(boost::dynamic_bitset<>(img.getHeight() * img.getWidth())) { // TODO: determine bounds
  for (size_t row_idx = 0; row_idx < img.getHeight(); row_idx++) {
    for (size_t col_idx = 0; col_idx < img.getWidth(); col_idx++) {
      this->setPixel(row_idx, col_idx,
                     img[row_idx, col_idx] == ImgAlpha::FILL_VALUE);
    }
  }
}
BitImage::BitImage(const size_t height, const size_t width)
    : height(height), width(width), data(boost::dynamic_bitset<>(height * width)) {}
BitImage::BitImage(BitImage &&other) noexcept : height(other.height), width(other.width), data(std::move(other.data)) {}

ptrdiff_t BitImage::getWidth() const { return this->width; }
ptrdiff_t BitImage::getHeight() const { return this->height; }

uint64_t BitImage::nPixels() const {
  uint64_t res = 0;
  for (size_t i = 0; i < this->getHeight(); i++) {
    for (size_t j = 0; j < this->getWidth(); j++) {
      if ((*this)[i, j]) {
        res++;
      }
    }
  }
  return res;
}

bool BitImage::operator[](const size_t i, const size_t j) const {
 return data[i * this->getWidth() + j];
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

void BitImage::setPixel(const size_t i, const size_t j, const bool value) {
  this->data[i * this->getWidth() + j] = value;
}
} // namespace gp
