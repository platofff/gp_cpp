#include <cstddef>
#ifdef _WIN32
#define NOMINMAX 1
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <mdspan>

#include "../include/genpattern.h"

static const size_t multiplier = 20;
static const uint8_t FILL_VALUE = 255;

TEST(DynamicCAPI, DenseFillCanvas) {
#ifdef _WIN32
    // Load the shared library for Windows.
    HMODULE handle = LoadLibraryA("genpattern.dll");
    ASSERT_NE(handle, nullptr) << "LoadLibrary failed: " << GetLastError();

    auto gp_genpattern_ptr = reinterpret_cast<decltype(&gp_genpattern)>(GetProcAddress(handle, "gp_genpattern"));
    ASSERT_NE(gp_genpattern_ptr, nullptr) << "GetProcAddress failed: " << GetLastError();
#else
    // Load the shared library with RTLD_LOCAL for non-Windows (e.g., Linux).
    void* handle = dlopen("./libgenpattern.so", RTLD_LAZY | RTLD_LOCAL);
    ASSERT_NE(handle, nullptr) << "dlopen failed: " << dlerror();

    dlerror(); // Clear any existing errors.
    auto gp_genpattern_ptr = reinterpret_cast<decltype(&gp_genpattern)>(dlsym(handle, "gp_genpattern"));
    const char* dlsym_error = dlerror();
    ASSERT_EQ(dlsym_error, nullptr) << "dlsym failed: " << dlsym_error;
#endif

    // Create a fully filled image: (10 * multiplier) x (10 * multiplier).
    size_t imgW = 10 * multiplier, imgH = 10 * multiplier;
    std::vector<uint8_t> fullData(imgW * imgH, FILL_VALUE);

    // Build an array of 9 GPImgAlpha images.
    const size_t n_images = 9;
    std::vector<GPImgAlpha> images(n_images);
    for (size_t i = 0; i < n_images; ++i) {
        images[i].width = imgW;
        images[i].height = imgH;
        images[i].data = fullData.data();
        // Initialize offsets to zero.
        for (ptrdiff_t j = 0; j < 4; ++j) {
            images[i].offsets[j].x = 0;
            images[i].offsets[j].y = 0;
        }
        images[i].offsets_size = 0;
    }

    // Wrap the images into a GPCollection.
    GPCollection collection;
    collection.n_images = n_images;
    collection.images = images.data();
    const size_t n_collections = 1;

    // Set canvas dimensions: (30 * multiplier) x (30 * multiplier).
    size_t canvasW = 30 * multiplier, canvasH = 30 * multiplier;
    uint8_t threshold = 64;
    size_t offset_radius = 0, collection_offset_radius = 0;

    // Setup an exponential cooling schedule with alpha 0.95.
    GPSchedule schedule;
    schedule.type = GP_SCHEDULE_EXPONENTIAL;
    schedule.params.exponential.alpha = 0.95;

    // Fixed seed.
    const uint32_t seed = 42;

    // Prepare an exception buffer.
    const size_t bufferSize = 1024;
    char exception_buffer[bufferSize];
    std::memset(exception_buffer, 0, bufferSize);

    // Invoke the C API function.
    ptrdiff_t result = gp_genpattern_ptr(&collection, n_collections,
                                   canvasW, canvasH,
                                   threshold,
                                   offset_radius, collection_offset_radius,
                                   &schedule, seed,
                                   exception_buffer, bufferSize);

    // Expect success (return code 0).
    EXPECT_EQ(result, 0) << "Expected return value 0 for success, but got "
                         << result << ". Exception: " << exception_buffer;

    // Create a virtual canvas as a 2D array using std::mdspan.
    // The first index is the row (y) and the second index is the column (x).
    std::vector<uint8_t> canvas(canvasW * canvasH, 0);
    std::mdspan<uint8_t, std::extents<size_t, std::dynamic_extent, std::dynamic_extent>>
        canvas_md(canvas.data(), canvasH, canvasW);

    // "Draw" each image onto the canvas for each placement offset.
    // This simulates painting the images while respecting the canvas boundaries.
    for (size_t i = 0; i < n_images; ++i) {
        const GPImgAlpha& img = images[i];
        for (size_t j = 0; j < img.offsets_size; ++j) {
            ptrdiff_t off_x = img.offsets[j].x;
            ptrdiff_t off_y = img.offsets[j].y;
            for (ptrdiff_t y = 0; y < img.height; ++y) {
                ptrdiff_t cy = off_y + y;
                if (cy < 0 || cy >= canvasH)
                    continue;
                for (ptrdiff_t x = 0; x < img.width; ++x) {
                    ptrdiff_t cx = off_x + x;
                    if (cx < 0 || cx >= canvasW)
                        continue;
                    canvas_md[cy, cx] = 1; // Mark pixel as filled.
                }
            }
        }
    }

    // Count the number of uniquely filled pixels on the canvas.
    ptrdiff_t filledPixels = std::count(canvas.begin(), canvas.end(), static_cast<uint8_t>(1));

    // Calculate the total effective area of all placed images.
    // For each image placement, compute the intersection of the image rectangle with the canvas.
    // Sum these effective areas; if there's no overlap, this should match the number of filled pixels.
    ptrdiff_t expectedFilled = 0;
    for (size_t i = 0; i < n_images; ++i) {
        const GPImgAlpha& img = images[i];
        for (size_t j = 0; j < img.offsets_size; ++j) {
            ptrdiff_t off_x = img.offsets[j].x;
            ptrdiff_t off_y = img.offsets[j].y;

            ptrdiff_t img_left   = off_x;
            ptrdiff_t img_top    = off_y;
            ptrdiff_t img_right  = off_x + static_cast<ptrdiff_t>(imgW);
            ptrdiff_t img_bottom = off_y + static_cast<ptrdiff_t>(imgH);

            // Compute intersection with the canvas [0, canvasW) x [0, canvasH).
            ptrdiff_t inter_left   = std::max(img_left, static_cast<ptrdiff_t>(0));
            ptrdiff_t inter_top    = std::max(img_top, static_cast<ptrdiff_t>(0));
            ptrdiff_t inter_right  = std::min(static_cast<ptrdiff_t>(canvasW), img_right);
            ptrdiff_t inter_bottom = std::min(static_cast<ptrdiff_t>(canvasH), img_bottom);

            ptrdiff_t inter_width  = inter_right - inter_left;
            ptrdiff_t inter_height = inter_bottom - inter_top;
            if (inter_width > 0 && inter_height > 0) {
                expectedFilled += inter_width * inter_height;
            }
        }
    }

    // Verify that the total number of unique filled pixels equals the sum of the areas
    // of all placed images, ensuring that no images overlap.
    EXPECT_EQ(filledPixels, expectedFilled)
         << "Expected total filled area of " << expectedFilled 
         << " pixels (sum of all placed images' effective areas), but got " 
         << filledPixels;

#ifdef _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}
