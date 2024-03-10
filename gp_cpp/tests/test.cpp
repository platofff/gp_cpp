#include <cstddef>

#include <Canvas.hpp>

using namespace gp;

constexpr size_t W = 16;
constexpr size_t H = 16;
uint8_t data[W * H] = {};

int main() {
  Canvas<uint8_t> canvas(64, 64);

  for (size_t i = 4; i < 12; i++) {
    for (size_t j = 4; j < 12; j++) {
      data[j * W + i] = 255;
    }
  }

  BitImage<uint8_t> img(ImgAlpha{data, W, H});
  Point pos = {3, 0};

  canvas.addImage(img, pos);

  int intersection = canvas.intersectionArea(img, pos);
  std::cout << "Intersection area: " << intersection << std::endl;
  canvas.debug();
}