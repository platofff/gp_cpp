#include "PatternGenerator.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "OffsettedBitImage.hpp"
#include "misc.hpp"

namespace gp {
	PatternGenerator::PatternGenerator(
		const size_t width, const size_t height,
		const std::vector<std::vector<ImgAlphaFilledContour>>& collections,
		const size_t offset, const size_t collection_offset,
		const double temperature_initial)
		: width(width), height(height),
		box(Box{ Point{0, 0}, Point{static_cast<ptrdiff_t>(width) - 1,
								   static_cast<ptrdiff_t>(height) - 1} }),
		temperatureInitial(temperature_initial) {
		if (width == 0 || height == 0) {
			throw std::invalid_argument("PatternGenerator: canvas dimensions must be non-zero");
		}
		if (collections.empty()) {
			throw std::invalid_argument("PatternGenerator: collections must not be empty");
		}
		for (const auto& coll : collections) {
			if (coll.empty()) {
				throw std::invalid_argument("PatternGenerator: each collection must contain at least one image");
			}
			for (const auto& img : coll) {
				if (img.getWidth() == 0 || img.getHeight() == 0) {
					throw std::invalid_argument("PatternGenerator: all images must have non-zero dimensions");
				}
				if (img.getWidth() >= width || img.getHeight() >= height) {
					throw std::invalid_argument("Pattern Generator: some of images is larger than the canvas");
				}
			}
		}

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
