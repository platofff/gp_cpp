#include "BitImage.hpp"
#include <cstddef>

namespace gp {
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
