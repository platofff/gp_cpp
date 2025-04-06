#include <gtest/gtest.h>
#include <random>
#include <vector>
#include "PatternGenerator.hpp"
#include "ImgAlphaFilledContour.hpp"
#include "Canvas.hpp"
#include "OffsettedBitImage.hpp"
#include "CoolingSchedules.hpp"
#include "misc.hpp"

using namespace gp;

static const size_t multiplier = 20;

// Test 1. Dense Canvas Fill
// This test creates a fully filled (all pixels set to FILL_VALUE) image of size (10*multiplier)x(10*multiplier)
// and builds a collection with 9 copies (using emplace to avoid copying).
// When these images are placed on a canvas of size (30*multiplier)x(30*multiplier),
// the sum of the filled pixel counts from each image (assuming non-overlapping placement)
// should equal the total number of filled pixels on the canvas.
// The PatternGenerator computes placements, which are then applied to a Canvas.
// Finally, the test verifies that the canvas has the expected total number of filled pixels.
TEST(PatternGeneratorTest, DenseFillCanvas) {
  // Create a (10*multiplier)x(10*multiplier) image with all pixels set to FILL_VALUE.
  const size_t imgW = 10 * multiplier, imgH = 10 * multiplier;
  std::vector<uint8_t> fullData(imgW * imgH, ImgAlpha::FILL_VALUE);
  
  // Create 9 independent ImgAlphaFilledContour objects using the same data.
  std::vector<ImgAlphaFilledContour> collection;
  collection.reserve(9);
  for (int i = 0; i < 9; i++) {
      collection.emplace_back(fullData.data(), imgW, imgH, 64);
  }
  
  // Build a collection (vector of collections), here using a single collection.
  std::vector<std::vector<ImgAlphaFilledContour>> collections;
  collections.push_back(std::move(collection));
  
  // Create a PatternGenerator for a (30*multiplier)x(30*multiplier) canvas.
  // Using offset and collectionOffset equal to 0 minimizes any extra transformation.
  size_t canvasW = 30 * multiplier, canvasH = 30 * multiplier;
  double temperatureInitial = 100.0;
  PatternGenerator pg(canvasW, canvasH, collections, 0, 0, temperatureInitial);
  
  // Use an exponential cooling schedule with a fixed parameter.
  ExponentialSchedule expSched{0.95};
  uint32_t seed = 42;
  auto placements = pg.generate(seed, expSched);
  
  // Create a Canvas to simulate the placement.
  std::mt19937 rng(seed);
  Canvas canvas(canvasW, canvasH, rng);
  
  uint64_t expectedFilled = 0;
  
  // 'placements' is a 3D vector: [collection][image][list of possible placements].
  // For each image that has at least one placement, add the full image area
  // to the expected filled pixel count, and place the image on the canvas.
  for (size_t collIdx = 0; collIdx < placements.size(); ++collIdx) {
      for (size_t imgIdx = 0; imgIdx < placements[collIdx].size(); ++imgIdx) {
          if (!placements[collIdx][imgIdx].empty()) {
              Point p = placements[collIdx][imgIdx][0];
              // Each image contributes imgW * imgH filled pixels.
              expectedFilled += imgH * imgW;
              // Reconstruct a new ImgAlphaFilledContour for each placement,
              // since the original objects have been moved into PatternGenerator.
              ImgAlphaFilledContour img(fullData.data(), imgW, imgH, 64);
              canvas.addImage(img, p);
          }
      }
  }
  
  EXPECT_EQ(canvas.nPixels(), expectedFilled);
}


// Test 2. Verify that applying an offset (OffsettedBitImage) increases filled bits.
TEST(OffsettedBitImageTest, OffsetAddsPixels) {
    const size_t W = 3 * multiplier, H = 3 * multiplier;
    std::vector<uint8_t> data(W * H, 0);
    // Only the center pixel is filled.
    data[(H/2) * W + (W/2)] = ImgAlpha::FILL_VALUE;
    ImgAlphaFilledContour img(data.data(), W, H, 64);
    
    // Create a BitImage from ImgAlpha (without offset)
    BitImage orig(img);
    uint64_t origCount = orig.nPixels();
    
    // Create an OffsettedBitImage with offset radius 1 (using generateDisk)
    auto disk = generateDisk(1);
    OffsettedBitImage offImg(img, disk, 1);
    uint64_t offCount = offImg.nPixels();
    
    EXPECT_GT(offCount, origCount);
}


// Test 3. Verify that invalid inputs are handled.
TEST(PatternGeneratorTest, InvalidInputs) {
    // Prepare valid data: a (3*multiplier)x(3*multiplier) image with the center pixel filled.
    const size_t W = 3 * multiplier, H = 3 * multiplier;
    std::vector<uint8_t> data(W * H, 0);
    data[(H/2) * W + (W/2)] = ImgAlpha::FILL_VALUE;
    
    // Build a collection with one image (using move semantics)
    std::vector<std::vector<ImgAlphaFilledContour>> collections;
    {
        std::vector<ImgAlphaFilledContour> vec;
        vec.emplace_back(data.data(), W, H, 64);
        collections.push_back(std::move(vec));
    }
    
    // Zero canvas width or height should throw an exception.
    EXPECT_THROW({
        PatternGenerator pg(0, 10 * multiplier, collections, 1, 1, 100.0);
    }, std::invalid_argument);
    EXPECT_THROW({
        PatternGenerator pg(10 * multiplier, 0, collections, 1, 1, 100.0);
    }, std::invalid_argument);
    
    // Empty collection should throw an exception.
    std::vector<std::vector<ImgAlphaFilledContour>> emptyCollections;
    EXPECT_THROW({
        PatternGenerator pg(10 * multiplier, 10 * multiplier, emptyCollections, 1, 1, 100.0);
    }, std::invalid_argument);
    
    // Image larger than the canvas: create a (4*multiplier)x(4*multiplier) image and a (3*multiplier)x(3*multiplier) canvas.
    const size_t bigW = 4 * multiplier, bigH = 4 * multiplier;
    std::vector<uint8_t> bigData(bigW * bigH, ImgAlpha::FILL_VALUE);
    ImgAlphaFilledContour bigImg(bigData.data(), bigW, bigH, 64);
    std::vector<std::vector<ImgAlphaFilledContour>> collectionsBig;
    {
        std::vector<ImgAlphaFilledContour> vec;
        vec.emplace_back(bigData.data(), bigW, bigH, 64);
        collectionsBig.push_back(std::move(vec));
    }
    EXPECT_THROW({
        PatternGenerator pg(3 * multiplier, 3 * multiplier, collectionsBig, 1, 1, 100.0);
    }, std::invalid_argument);
    
    // Zero threshold for ImgAlphaFilledContour should throw an exception.
    EXPECT_THROW({
        ImgAlphaFilledContour invalidImg(data.data(), W, H, 0);
    }, std::invalid_argument);
}


// Test 4. Verify that both cooling schedules return non-empty results,
// and that for each image there is at least one placement with non-negative coordinates
// and within the canvas bounds.
TEST(PatternGeneratorTest, CoolingSchedules) {
    // Create a fully filled (3*multiplier)x(3*multiplier) image.
    const size_t W = 3 * multiplier, H = 3 * multiplier;
    std::vector<uint8_t> data(W * H, ImgAlpha::FILL_VALUE);
    ImgAlphaFilledContour img(data.data(), W, H, 64);
    
    // Build a collection with 3 copies using emplace.
    std::vector<std::vector<ImgAlphaFilledContour>> collections;
    {
        std::vector<ImgAlphaFilledContour> vec;
        for (int i = 0; i < 3; i++) {
            vec.emplace_back(data.data(), W, H, 64);
        }
        collections.push_back(std::move(vec));
    }
    
    size_t canvasW = 10 * multiplier, canvasH = 10 * multiplier;
    double temperatureInitial = 100.0;
    PatternGenerator pg(canvasW, canvasH, collections, 1, 1, temperatureInitial);
    
    uint32_t seed = 42;
    // Exponential cooling schedule.
    ExponentialSchedule expSched{0.80};
    auto resultExp = pg.generate(seed, expSched);
    ASSERT_FALSE(resultExp.empty());
    // For each image's placements, ensure that at least one placement is within canvas bounds.
    for (const auto &collRes : resultExp) {
        for (const auto &placements : collRes) {
            EXPECT_FALSE(placements.empty());
            bool validPlacementFound = false;
            for (const auto &pt : placements) {
                if (pt.getX() >= 0 && pt.getY() >= 0 &&
                    pt.getX() < static_cast<ptrdiff_t>(canvasW) &&
                    pt.getY() < static_cast<ptrdiff_t>(canvasH)) {
                    validPlacementFound = true;
                    break;
                }
            }
            EXPECT_TRUE(validPlacementFound) << "No valid placement within canvas bounds found.";
        }
    }
    
    // Linear cooling schedule.
    LinearSchedule linSched{0.90};
    auto resultLin = pg.generate(seed, linSched);
    ASSERT_FALSE(resultLin.empty());
    for (const auto &collRes : resultLin) {
        for (const auto &placements : collRes) {
            EXPECT_FALSE(placements.empty());
            bool validPlacementFound = false;
            for (const auto &pt : placements) {
                if (pt.getX() >= 0 && pt.getY() >= 0 &&
                    pt.getX() < static_cast<ptrdiff_t>(canvasW) &&
                    pt.getY() < static_cast<ptrdiff_t>(canvasH)) {
                    validPlacementFound = true;
                    break;
                }
            }
            EXPECT_TRUE(validPlacementFound) << "No valid placement within canvas bounds found (linear schedule).";
        }
    }
}
