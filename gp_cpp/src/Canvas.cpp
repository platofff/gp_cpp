#include "Canvas.hpp"
#include "BitImage.hpp"

#include <bit>
#include <random>
#include <stdexcept>

namespace gp {
std::vector<PlacementArea>
Canvas::placementAreas(const BitImage &img, const Point pos) const noexcept {
  assert(img.getHeight() > this->getHeight() &&
         img.getWidth() > this->getWidth());
  assert(pos.getX() >= 0 && pos.getX() < this->getWidth());
  assert(pos.getY() >= 0 && pos.getY() < this->getHeight());

  std::vector<PlacementArea> out;
  out.reserve(4);

  const auto imgWidth = static_cast<ptrdiff_t>(img.getWidth());
  const auto imgHeight = static_cast<ptrdiff_t>(img.getHeight());
  const Box bounds = {pos,
                      {pos.getX() + imgWidth - 1, pos.getY() + imgHeight - 1}};

  for (int i = BOTTOM; i != AREAS_SIZE; i++) {
    const Box intersection = bounds.intersect(this->areas[i]);
    if (intersection.isValid()) {
      out.emplace_back(
          intersection.translate(this->offsets[i]),
          intersection.getMin().translate(this->offsets[i]),
          Point{intersection.getMin().getX() - bounds.getMin().getX(),
                intersection.getMin().getY() - bounds.getMin().getY()});
      assert(out.size() <= 4);
    }
  }
  return out;
}

Canvas::Canvas(const ptrdiff_t width, const ptrdiff_t height, std::mt19937 &rng)
    : BitImage(height, width), // use the BitImage(height,width) constructor for
                               // an empty canvas
      areas{{{{0, height}, {width - 1, nlp::max()}},
             {{width, 0}, {nlp::max(), height - 1}},
             {{width, height}, {nlp::max(), nlp::max()}},
             {{0, 0}, {width - 1, height - 1}}}},
      offsets{{{0, -height}, {-width, 0}, {-width, -height}, {0, 0}}},
      deltaMaxInitial{std::ceil(static_cast<double>(width) / 2.0),
                      std::ceil(static_cast<double>(height) / 2.0)},
      rng{rng} {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Canvas: width and height must be positive");
  }
}

Canvas::Canvas(Canvas &&other)
    : BitImage(static_cast<BitImage &&>(other)), areas(std::move(other.areas)),
      offsets(std::move(other.offsets)),
      deltaMaxInitial(std::move(other.deltaMaxInitial)), rng(other.rng) {}

// optimized by OpenAI o1-preview
uint64_t Canvas::intersectionArea(const BitImage &img,
                                  const Point &pos) const noexcept {
  uint64_t res = 0;
  const auto areas = this->placementAreas(img, pos);

  // Access the underlying data blocks of the canvas and the image
  const std::vector<Block> &canvas_data = this->getData();
  const std::vector<Block> &image_data = img.getData();

  // Get the widths of the canvas and image in bits
  const size_t canvas_row_width = this->getWidth();
  const size_t image_row_width = img.getWidth();

  // Iterate over each placement area where the image overlaps the canvas
  for (const auto &pa : areas) {
    const ptrdiff_t image_start_y = pa.imageStart.getY();
    const ptrdiff_t canvas_start_y = pa.canvasStart.getY();
    const size_t height = pa.bounds.getHeight();
    const size_t width = pa.bounds.getWidth();

    // Iterate over each row within the overlapping area
    for (ptrdiff_t row = 0; row < height; ++row) {
      const ptrdiff_t i = image_start_y + row;   // Current row in the image
      const ptrdiff_t ci = canvas_start_y + row; // Current row in the canvas

      // Compute the bit offsets for the start of this row in both images
      const ptrdiff_t canvas_bit_offset =
          ci * canvas_row_width + pa.canvasStart.getX();
      const ptrdiff_t image_bit_offset =
          i * image_row_width + pa.imageStart.getX();

      // Ensure offsets are non-negative
      if (canvas_bit_offset < 0 || image_bit_offset < 0)
        continue;

      size_t num_bits = width; // Number of bits to process in this row

      // Initialize block indices and bit offsets within those blocks
      size_t canvas_block_index = canvas_bit_offset / bits_per_block;
      size_t canvas_bit_in_block = canvas_bit_offset % bits_per_block;
      size_t image_block_index = image_bit_offset / bits_per_block;
      size_t image_bit_in_block = image_bit_offset % bits_per_block;

      // Process the bits in chunks, handling block boundaries
      while (num_bits > 0) {
        // Calculate the maximum number of bits we can process in this chunk
        size_t canvas_remaining_bits = bits_per_block - canvas_bit_in_block;
        size_t image_remaining_bits = bits_per_block - image_bit_in_block;
        size_t chunk_size = std::min(
            num_bits, std::min(canvas_remaining_bits, image_remaining_bits));

        // Check bounds to prevent out-of-range access
        if (canvas_block_index >= canvas_data.size() ||
            image_block_index >= image_data.size()) {
          break; // Exit the loop if we're beyond the data size
        }

        // Extract the current blocks from the canvas and image data
        Block canvas_block = canvas_data[canvas_block_index];
        Block image_block = image_data[image_block_index];

        // Create a mask for the bits we're interested in
        Block mask;
        if (chunk_size < bits_per_block) {
          mask = (Block(1) << chunk_size) - 1;
        } else {
          mask = ~Block(0); // All bits set to 1
        }

        // Shift and mask the blocks to isolate the relevant bits
        Block canvas_chunk = (canvas_block >> canvas_bit_in_block) & mask;
        Block image_chunk = (image_block >> image_bit_in_block) & mask;

        // Compute the intersection of the bits
        Block intersection = canvas_chunk & image_chunk;

        // Count the number of set bits in the intersection
        res += std::popcount(intersection);

        // Update the number of bits left to process
        num_bits -= chunk_size;

        // Advance the bit offsets and block indices as needed
        canvas_bit_in_block += chunk_size;
        if (canvas_bit_in_block >= bits_per_block) {
          canvas_bit_in_block = 0;
          ++canvas_block_index;
        }

        image_bit_in_block += chunk_size;
        if (image_bit_in_block >= bits_per_block) {
          image_bit_in_block = 0;
          ++image_block_index;
        }
      }
    }
  }
  return res;
}

// optimized by OpenAI o1-preview
void Canvas::addImage(const BitImage &img, const Point &pos) noexcept {
  const auto areas = this->placementAreas(img, pos);

  // Access the underlying data blocks of the canvas and the image
  std::vector<Block> &canvas_data = this->data;
  const std::vector<Block> &image_data = img.getData();

  // Get the widths of the canvas and image in bits
  const size_t canvas_row_width = this->getWidth();
  const size_t image_row_width = img.getWidth();

  // Iterate over each placement area where the image overlaps the canvas
  for (const auto &pa : areas) {
    const ptrdiff_t image_start_y = pa.imageStart.getY();
    const ptrdiff_t canvas_start_y = pa.canvasStart.getY();
    const size_t height = pa.bounds.getHeight();
    const size_t width = pa.bounds.getWidth();

    // Iterate over each row within the overlapping area
    for (ptrdiff_t row = 0; row < height; ++row) {
      const ptrdiff_t i = image_start_y + row;   // Current row in the image
      const ptrdiff_t ci = canvas_start_y + row; // Current row in the canvas

      // Compute the bit offsets for the start of this row in both images
      const ptrdiff_t canvas_bit_offset =
          ci * canvas_row_width + pa.canvasStart.getX();
      const ptrdiff_t image_bit_offset =
          i * image_row_width + pa.imageStart.getX();

      // Ensure offsets are non-negative
      if (canvas_bit_offset < 0 || image_bit_offset < 0)
        continue;

      size_t num_bits = width; // Number of bits to process in this row

      // Initialize block indices and bit offsets within those blocks
      size_t canvas_block_index = canvas_bit_offset / bits_per_block;
      size_t canvas_bit_in_block = canvas_bit_offset % bits_per_block;
      size_t image_block_index = image_bit_offset / bits_per_block;
      size_t image_bit_in_block = image_bit_offset % bits_per_block;

      // Process the bits in chunks, handling block boundaries
      while (num_bits > 0) {
        // Calculate the maximum number of bits we can process in this chunk
        size_t canvas_remaining_bits = bits_per_block - canvas_bit_in_block;
        size_t image_remaining_bits = bits_per_block - image_bit_in_block;
        size_t chunk_size = std::min(
            num_bits, std::min(canvas_remaining_bits, image_remaining_bits));

        // Check bounds to prevent out-of-range access
        if (canvas_block_index >= canvas_data.size() ||
            image_block_index >= image_data.size()) {
          break; // Exit the loop if we're beyond the data size
        }

        // Extract the current blocks from the canvas and image data
        Block canvas_block = canvas_data[canvas_block_index];
        Block image_block = image_data[image_block_index];

        // Create a mask for the bits we're interested in
        Block mask;
        if (chunk_size < bits_per_block) {
          mask = (Block(1) << chunk_size) - 1;
        } else {
          mask = ~Block(0); // All bits set to 1
        }

        // Shift and mask the blocks to isolate the relevant bits
        Block canvas_chunk = (canvas_block >> canvas_bit_in_block) & mask;
        Block image_chunk = (image_block >> image_bit_in_block) & mask;

        // Combine the bits using bitwise OR to "add" the image onto the canvas
        Block combined_chunk = canvas_chunk | image_chunk;

        // Clear the bits in the canvas block where we're about to write
        Block clear_mask = ~(mask << canvas_bit_in_block);
        canvas_block = (canvas_block & clear_mask) |
                       (combined_chunk << canvas_bit_in_block);

        // Update the canvas data block
        canvas_data[canvas_block_index] = canvas_block;

        // Update the number of bits left to process
        num_bits -= chunk_size;

        // Advance the bit offsets and block indices as needed
        canvas_bit_in_block += chunk_size;
        if (canvas_bit_in_block >= bits_per_block) {
          canvas_bit_in_block = 0;
          ++canvas_block_index;
        }

        image_bit_in_block += chunk_size;
        if (image_bit_in_block >= bits_per_block) {
          image_bit_in_block = 0;
          ++image_block_index;
        }
      }
    }
  }
}

Point Canvas::wrapPosition(const ptrdiff_t x,
                           const ptrdiff_t y) const noexcept {
  return Point{((x % this->getWidth()) + this->getWidth()) % this->getWidth(),
               ((y % this->getHeight()) + this->getHeight()) %
                   this->getHeight()};
};
} // namespace gp
