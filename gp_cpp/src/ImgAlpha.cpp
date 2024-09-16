#include "ImgAlpha.hpp"
#include "misc.hpp"

#include <cstdint>

namespace gp {
ImgAlpha::ImgAlpha(const uint8_t *data, const size_t width, const size_t height)
    : alpha(make_aligned_mdarray<uint8_t>(height, width)) {
  if (data != nullptr) {
    std::memcpy(this->alpha.data_handle(), data, this->alpha.size());
  }
}

ImgAlpha::ImgAlpha(ImgAlpha &&other) noexcept : alpha(std::move(other.alpha)) {}

uint8_t &ImgAlpha::operator[](const size_t i, const size_t j) const {
  return this->alpha[i, j];
}

size_t ImgAlpha::getWidth() const { return this->alpha.extent(1); }
size_t ImgAlpha::getHeight() const { return this->alpha.extent(0); }
} // namespace gp
