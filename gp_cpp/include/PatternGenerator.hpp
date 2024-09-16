#pragma once

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "ImgAlphaFilledContour.hpp"
#include <random>

namespace gp {
class PatternGenerator {
private:
  std::vector<std::vector<BitImage>>
      oCollections, // collections of images without offset
      rCollections, // collections of images with regular offset
      sCollections; // with increased offset
  std::vector<std::vector<std::pair<Vector, Vector>>>
      baseOffsets; // baseOffsets for images with regular and increased offsets
  std::vector<Canvas> cCanvases; // canvas versions for each collection

  const ptrdiff_t width;
  const ptrdiff_t height;
  const Box box;
  std::random_device randomDev;
  std::mt19937 rng;

  std::vector<Point> getPlacementPoints(const Point &p,
                                        const ptrdiff_t img_width,
                                        const ptrdiff_t img_height) const;

public:
  PatternGenerator(const size_t width, const size_t height,
                   const std::vector<std::vector<ImgAlphaFilledContour>> &collections,
                   const size_t offset, const size_t collection_offset);

  std::vector<std::vector<std::vector<Point>>> generate();
};
} // namespace gp
