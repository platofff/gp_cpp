#include "PatternGenerator.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "OffsettedBitImage.hpp"
#include "misc.hpp"

namespace gp {
std::vector<Point>
PatternGenerator::getPlacementPoints(const Point &p, const ptrdiff_t img_width,
                                     const ptrdiff_t img_height) const {
  const std::array<Point, 4> points = {
      {p,
       {p.getX() - this->width, p.getY()},
       {p.getX(), p.getY() - this->height},
       {p.getX() - this->width, p.getY() - this->height}}};
  std::vector<Point> result;
  result.reserve(4);

  for (const auto &point : points) {
    Box img_box = {point,
                   Point{point.getX() + img_width, point.getY() + img_height}};
    if (img_box.intersect(this->box).isValid()) {
      result.push_back(point);
    }
  }

  return result;
}

PatternGenerator::PatternGenerator(
    const size_t width, const size_t height,
    const std::vector<std::vector<ImgAlphaFilledContour>> &collections,
    const size_t offset, const size_t collection_offset,
    const double temperature_initial)
    : width(width), height(height),
      box(Box{Point{0, 0}, Point{static_cast<ptrdiff_t>(width) - 1,
                                 static_cast<ptrdiff_t>(height) - 1}}),
      temperatureInitial(temperature_initial) {
  const size_t nCollections = collections.size();
  const auto rDisk = generateDisk(offset);
  const auto sDisk = generateDisk(collection_offset);

  for (size_t i = 0; i < nCollections; i++) {
    const size_t nImages = collections[i].size();

    std::vector<BitImage> oCol;
    std::vector<OffsettedBitImage> rCol, sCol;
    rCol.reserve(nImages);
    sCol.reserve(nImages);
    oCol.reserve(nImages);

    for (size_t j = 0; j < nImages; j++) {
      rCol.emplace_back(collections[i][j], rDisk, offset);
      sCol.emplace_back(collections[i][j], sDisk, collection_offset);
      oCol.emplace_back(collections[i][j]);
    }

    this->rCollections.push_back(std::move(rCol));
    this->sCollections.push_back(std::move(sCol));
    this->oCollections.push_back(std::move(oCol));
  }
}
} // namespace gp
