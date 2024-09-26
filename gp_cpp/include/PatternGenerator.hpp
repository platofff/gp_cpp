#pragma once

#include "BitImage.hpp"
#include "Canvas.hpp"
#include "ImgAlphaFilledContour.hpp"

#include <random>
#include <algorithm>

namespace gp {
class PatternGenerator {
private:
  std::vector<std::vector<BitImage>>
      oCollections, // collections of images without offset
      rCollections, // collections of images with regular offset
      sCollections; // with increased offset
  std::vector<std::vector<std::pair<Vector, Vector>>>
      baseOffsets; // baseOffsets for images with regular and increased offsets

  const ptrdiff_t width;
  const ptrdiff_t height;
  const double temperatureInitial;
  const Box box;

  std::vector<Point> getPlacementPoints(const Point &p,
                                        const ptrdiff_t img_width,
                                        const ptrdiff_t img_height) const;

public:
  PatternGenerator(
      const size_t width, const size_t height,
      const std::vector<std::vector<ImgAlphaFilledContour>> &collections,
      const size_t offset, const size_t collectionOffset,
      const double temperatureInitial);

  template <typename CoolingSchedule> 
  std::vector<std::vector<std::vector<Point>>>
  generate(const uint32_t seed, const CoolingSchedule decreaseT) const {
    std::mt19937 rng(seed);
    std::vector<std::pair<size_t, size_t>> indices;
    std::vector<Canvas> cCanvases; // canvas versions for each collection

    const size_t nCollections = this->rCollections.size();
    cCanvases.reserve(nCollections);

    std::vector<std::vector<std::vector<Point>>> result(nCollections);

    for (size_t i = 0; i < nCollections; i++) {
      cCanvases.emplace_back(this->width, this->height, rng);
      const size_t nImages = this->rCollections[i].size();
      result[i] = std::vector<std::vector<Point>>(nImages);
      for (size_t j = 0; j < nImages; j++) {
        indices.push_back(std::make_pair(i, j));
      }
    }

    std::shuffle(indices.begin(), indices.end(), rng);

    for (const auto &[collection_idx, img_idx] : indices) {
      const auto &img = oCollections[collection_idx][img_idx];
      const auto _p = cCanvases[collection_idx].optimizePlacement(
          img, temperatureInitial, decreaseT);
      if (_p.has_value()) {
        const auto &p = *_p;
        const auto [bo, sbo] = this->baseOffsets[collection_idx][img_idx];
        result[collection_idx][img_idx] =
            this->getPlacementPoints(p, img.getWidth(), img.getHeight());

        Point rPos{p.getX() + bo.getX(), p.getY() + bo.getY()};
        Point sPos{p.getX() + sbo.getX(), p.getY() + sbo.getY()};

        for (size_t i = 0; i < nCollections; i++) {
          if (i != collection_idx) {
            cCanvases[i].addImage(this->rCollections[collection_idx][img_idx],
                                  rPos, true);
          } else {
            cCanvases[i].addImage(this->sCollections[collection_idx][img_idx],
                                  sPos, false);
          }
        }
      }
    }

    return std::move(result);
  }
};
} // namespace gp
