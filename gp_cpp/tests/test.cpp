#include "ImgAlphaFilledContour.hpp"
#include "PatternGenerator.hpp"
#include <cstddef>

#include <Canvas.hpp>
#include <vector>

using namespace gp;

constexpr size_t W = 16;
constexpr size_t H = 16;
uint8_t data[W * H] = {};

int main() {

  for (size_t i = 3; i < W - 3; i++) {
    for (size_t j = 1; j < H - 3; j++) {
      data[j * W + i] = 255;
    }
  }

  /*
  BitImage img(ImgAlpha{data, W, H});

  Canvas canvas(4, 4);
  canvas.addImage(img, {0, 2});


  for (ptrdiff_t i = 0; i < 64; i++) {
    for (ptrdiff_t j = 0; j < 64; j++) {
      std::cout << i << " " << j << std::endl;
      Canvas canvas(64, 64);
      canvas.addImage(img, {i, j});
    }
  }*/

  std::vector<std::vector<ImgAlphaFilledContour>> collections;
  collections.emplace_back();
  collections[0].emplace_back(data, W, H, 64);
  collections[0].emplace_back(data, W, H, 64);
  collections[0].emplace_back(data, W, H, 64);

  /*
  PatternGenerator generator(128, 128, collections, 0, 0);
  auto result = generator.generate();

  for (const auto &c : result) {
    for (const auto &c1 : c) {
      for (const auto &p : c1) {
        std::cout << p << std::endl;
      }
    }
  }*/
}