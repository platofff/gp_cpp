#include <cstddef>

#include <Canvas.hpp>

using namespace gp;

constexpr size_t W = 15;
constexpr size_t H = 17;
uint8_t data[W * H] = {};

int main() {
  for (size_t i = 0; i < W; i++) {
    for (size_t j = 0; j < H; j++) {
      data[j * W + i] = 255;
    }
  }

  BitImage<uint8_t> img(ImgAlpha{data, W, H});

  /*
  Canvas<uint8_t> canvas(128, 128);
  canvas.addImage(img, {7, 0});
  canvas.debug();*/
  

  
  for (ptrdiff_t i = 0; i < 64; i++) {
    for (ptrdiff_t j = 0; j < 64; j++) {
      std::cout << i << " " << j << std::endl;
      Canvas<uint8_t> canvas(64, 64);
      canvas.addImage(img, {i, j});
    }
  }
}