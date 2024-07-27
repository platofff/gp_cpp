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

ImgAlpha::ImgAlpha(){};

ImgAlpha::~ImgAlpha() = default;

void ImgAlpha::generateAndFillContour(uint8_t threshold) {
  const auto fill_buf =
      make_aligned_unique_array<PixelState>(this->alpha.size());
  const auto fill =
      std::mdspan(fill_buf.get(), this->getHeight(), this->getWidth());

  bool no_start_points = true;

  std::vector<Point> fill_start_points;
  this->getFilteredPerimeter(
      fill_start_points, [threshold](uint8_t val) { return val < threshold; });

  for (const auto &p : fill_start_points) {
    no_start_points = false;
    std::stack<Point> point_stack;

    const auto push_if_valid = [&point_stack, &fill, this](const Point &p) {
      if (p.getY() >= 0 && p.getY() < this->getHeight() && p.getX() >= 0 &&
          p.getX() < this->getWidth() && fill[p.getY(), p.getX()] == PixelState::NOT_CHECKED) {
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
  if (no_start_points) {
    this->getFilteredPerimeter(this->contour);
    std::fill_n(this->alpha.data_handle(), this->alpha.size(), ImgAlpha::FILL_VALUE);
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

} // namespace gp
