#include "ImgAlpha.hpp"
#include "misc.hpp"

#include <cstdint>

namespace gp {
ImgAlpha::ImgAlpha(const uint8_t *data, const size_t width, const size_t height)
    : alpha(make_aligned_mdarray<uint8_t>(height, width)) {
  if (data != nullptr) {
    std::memcpy(this->alpha.data_handle(), data, this->alpha.size());
  } else {
    throw std::invalid_argument("ImgAlpha: data pointer is null");
  }
}

ImgAlpha::ImgAlpha(ImgAlpha &&other) noexcept : alpha(std::move(other.alpha)) {}

std::ostream &operator<<(std::ostream &stream, const ImgAlpha &img) {
  for (size_t i = 0; i < img.getWidth(); i++) {
    for (size_t j = 0; j < img.getHeight(); j++) {
      stream << static_cast<int>(img[i, j] == img.FILL_VALUE);
    }
    stream << std::endl;
  }
  return stream;
}
} // namespace gp
