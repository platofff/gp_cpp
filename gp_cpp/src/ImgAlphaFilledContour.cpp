#include "ImgAlphaFilledContour.hpp"
#include "ImgAlpha.hpp"

#include <memory>
#include <stack>

namespace gp {
	void ImgAlphaFilledContour::generateAndFillContour(const uint8_t threshold) {
		const auto fill_buf =
			make_aligned_unique_array<PixelState>(this->alpha.size());
		const auto fill =
			std::mdspan(fill_buf.get(), this->getHeight(), this->getWidth());

		auto fill_start_points = this->getFilteredPerimeter<std::vector<Point>>(
			[threshold](uint8_t val) { return val < threshold; });

		for (const auto& p : fill_start_points) {
			std::stack<Point> point_stack;

			const auto push_if_valid = [&point_stack, &fill, this](const Point& p) {
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

					push_if_valid(Point{ x + 1, y });
					push_if_valid(Point{ x, y + 1 });
					push_if_valid(Point{ x - 1, y });
					push_if_valid(Point{ x, y - 1 });
				}
			}
		}

		if (fill_start_points.empty()) {
			std::fill_n(this->alpha.data_handle(), this->alpha.size(),
				ImgAlpha::FILL_VALUE);
			return;
		}

		for (ptrdiff_t i = 0; i < this->getHeight(); i++) {
			for (ptrdiff_t j = 0; j < this->getWidth(); j++) {
				if (fill[i, j] == PixelState::NOT_CHECKED) {
					(*this)[i, j] = ImgAlpha::FILL_VALUE;
				}
			}
		}
	}

	ImgAlphaFilledContour::ImgAlphaFilledContour(const uint8_t* data,
		const size_t width,
		const size_t height,
		const uint8_t threshold)
		: ImgAlpha(data, width, height) {
		if (width == 0 || height == 0) {
			throw std::invalid_argument("ImgAlphaFilledContour: image dimensions must be non-zero");
		}
		if (threshold == 0) {
			throw std::invalid_argument("ImgAlphaFilledContour: threshold must be > 0");
		}
		this->generateAndFillContour(threshold);
	}

	ImgAlphaFilledContour::ImgAlphaFilledContour(
		ImgAlphaFilledContour&& other) noexcept
		: ImgAlpha(static_cast<ImgAlpha&&>(other)) {
	}
} // namespace gp