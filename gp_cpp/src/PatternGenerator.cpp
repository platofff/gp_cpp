#include "PatternGenerator.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "OffsettedImgAlpha.hpp"


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
    const double temperatureInitial)
    : width(width), height(height),
      box(Box{Point{0, 0}, Point{static_cast<ptrdiff_t>(width) - 1,
                                 static_cast<ptrdiff_t>(height) - 1}}),
      temperatureInitial(temperatureInitial){
  const size_t nCollections = collections.size();

  this->rCollections.reserve(nCollections);
  this->sCollections.reserve(nCollections);
  this->oCollections.reserve(nCollections);
  this->baseOffsets.reserve(nCollections);

  for (size_t i = 0; i < nCollections; i++) {
    const size_t nImages = collections[i].size();

    std::vector<BitImage> r_col, s_col, o_col;
    std::vector<std::pair<Vector, Vector>> bo;
    r_col.reserve(nImages);
    s_col.reserve(nImages);
    o_col.reserve(nImages);
    bo.reserve(nImages);

    for (size_t j = 0; j < nImages; j++) {
      const OffsettedImgAlpha r_img(collections[i][j], offset);
      const OffsettedImgAlpha s_img(collections[i][j], collection_offset);

      r_col.emplace_back(r_img);

      s_col.emplace_back(s_img);
      o_col.emplace_back(collections[i][j]);
      bo.emplace_back(
          std::make_pair(r_img.getBaseOffset(), s_img.getBaseOffset()));
    }

    this->rCollections.push_back(std::move(r_col));
    this->sCollections.push_back(std::move(s_col));
    this->oCollections.push_back(std::move(o_col));
    this->baseOffsets.push_back(std::move(bo));
  }
}
} // namespace gp
