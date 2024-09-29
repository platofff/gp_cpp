#include "misc.hpp"

namespace gp {
std::ostream &operator<<(std::ostream &stream, const Box &box) {
  return stream << "Box{" << box.getMin() << " " << box.getMax() << "}";
}

const aligned_mdarray<bool, 2> generateDisk(const ptrdiff_t r) {
  const size_t n = 2 * r + 1;
  auto disk = make_aligned_mdarray<bool>(n, n);

  for (ptrdiff_t i = 0; i < n; i++) {
    for (ptrdiff_t j = 0; j < n; j++) {
      const auto x = static_cast<double>(j - r);
      const auto y = static_cast<double>(i - r);
      const auto d = std::sqrt(x * x + y * y);
      if (d <= static_cast<double>(r)) {
        disk[i, j] = true;
      }
    }
  }

  return disk;
}

} // namespace gp