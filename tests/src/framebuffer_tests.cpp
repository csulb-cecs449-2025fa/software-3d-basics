#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "framebuffer.h"

TEST(Framebuffer, StoresPixelsInRowMajorOrder) {
	// The upload path treats the framebuffer as consecutive rows of RGBA pixels.
	const Pixel topLeft{ 1, 0, 0, 255 };
	const Pixel topRight{ 2, 0, 0, 255 };
	const Pixel bottomLeft{ 3, 0, 0, 255 };
	const Pixel bottomRight{ 4, 0, 0, 255 };
	Framebuffer framebuffer{ 2, 2 };

	framebuffer.setPixel(glm::ivec2{ 0, 0 }, topLeft);
	framebuffer.setPixel(glm::ivec2{ 1, 0 }, topRight);
	framebuffer.setPixel(glm::ivec2{ 0, 1 }, bottomLeft);
	framebuffer.setPixel(glm::ivec2{ 1, 1 }, bottomRight);

	const std::vector<Pixel> expectedPixels{
		topLeft, topRight, bottomLeft, bottomRight
	};
	EXPECT_EQ(framebuffer.data(), expectedPixels);
}

TEST(Framebuffer, ClearReplacesEveryPixel) {
	// Clearing must remove old geometry everywhere before the next frame is drawn.
	const Pixel firstColor{ 10, 20, 30, 255 };
	const Pixel secondColor{ 40, 50, 60, 128 };
	Framebuffer framebuffer{ 3, 2, firstColor };

	framebuffer.setPixel(glm::ivec2{ 1, 0 }, Pixel{ 90, 80, 70, 255 });
	framebuffer.clear(secondColor);

	for (const Pixel pixel : framebuffer.data()) {
		EXPECT_EQ(pixel, secondColor);
	}
}

TEST(Framebuffer, RejectsInvalidDimensions) {
	// A zero-sized framebuffer cannot be presented or indexed safely.
	EXPECT_THROW(Framebuffer(0, 2), std::invalid_argument);
	EXPECT_THROW(Framebuffer(2, 0), std::invalid_argument);
	EXPECT_THROW(Framebuffer(-1, 2), std::invalid_argument);
}
