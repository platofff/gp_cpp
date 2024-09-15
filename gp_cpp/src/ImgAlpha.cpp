#include "ImgAlpha.hpp"
#include "misc.hpp"

#include <cstdint>
#include <memory>
#include <stack>

namespace gp {
ImgAlpha::ImgAlpha(const uint8_t *data, const size_t width, const size_t height)
    : alpha(make_aligned_mdarray<uint8_t>(height, width)) {
  if (data != nullptr) {
    std::memcpy(this->alpha.data_handle(), data, this->alpha.size());
  }
}

void ImgAlpha::generateAndFillContour(const uint8_t threshold) {
  const auto fill_buf =
      make_aligned_unique_array<PixelState>(this->alpha.size());
  const auto fill =
      std::mdspan(fill_buf.get(), this->getHeight(), this->getWidth());

  auto fill_start_points = this->getFilteredPerimeter<std::vector<Point>>(
      [threshold](uint8_t val) { return val < threshold; });

  for (const auto &p : fill_start_points) {
    std::stack<Point> point_stack;

    const auto push_if_valid = [&point_stack, &fill, this](const Point &p) {
      if (p.getY() >= 0 && p.getY() < this->getHeight() && p.getX() >= 0 &&
          p.getX() < this->getWidth() &&
          fill[p.getY(), p.getX()] == PixelState::NOT_CHECKED) {
        point_stack.push(p);
      }
    };

    push_if_valid(p);

    while (!point_stack.empty()) {
      const auto point = point_stack.top();
      const auto x = point.getX();
      const auto y = point.getY();
      point_stack.pop();

      if ((*this)[y, x] < threshold) {
        fill[y, x] = PixelState::FILLED;

        push_if_valid(Point{x + 1, y});
        push_if_valid(Point{x, y + 1});
        push_if_valid(Point{x - 1, y});
        push_if_valid(Point{x, y - 1});
      } else {
        fill[y, x] = PixelState::CONTOUR;
      }
    }
  }

  this->contour.clear();
  if (fill_start_points.empty()) {
    this->contour = this->getFilteredPerimeter<std::vector<Point>>();
    std::fill_n(this->alpha.data_handle(), this->alpha.size(),
                ImgAlpha::FILL_VALUE);
    return;
  }

  for (ptrdiff_t i = 0; i < this->getHeight(); i++) {
    for (ptrdiff_t j = 0; j < this->getWidth(); j++) {
      if (fill[i, j] == PixelState::CONTOUR) {
        this->contour.emplace_back(j, i);
        (*this)[i, j] = ImgAlpha::FILL_VALUE;
      } else if (fill[i, j] == PixelState::NOT_CHECKED) {
        (*this)[i, j] = ImgAlpha::FILL_VALUE;
      }
    }
  }
}

const std::vector<Point> &ImgAlpha::getContour() const { return this->contour; }

ImgAlpha::ImgAlpha(ImgAlpha &&other) noexcept
    : alpha(std::move(other.alpha)), contour(std::move(other.contour)) {}

uint8_t &ImgAlpha::operator[](const size_t i, const size_t j) const {
  return this->alpha[i, j];
}

size_t ImgAlpha::getWidth() const { return this->alpha.extent(1); }
size_t ImgAlpha::getHeight() const { return this->alpha.extent(0); }
} // namespace gp
