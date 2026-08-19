#include <gtest/gtest.h>

#include <set>

#include "lines.h"
#include "raster_test_helpers.h"

namespace {

const Pixel background{};
const Pixel lineColor{ 255, 20, 40, 255 };

TEST(BresenhamLine, DrawsLowSlopeLine) {
	// A shallow line should contain both endpoints and the expected staircase.
	Framebuffer framebuffer{ 8, 8, background };
	drawLine(framebuffer, glm::ivec2{ 1, 1 }, glm::ivec2{ 6, 3 }, lineColor);

	const std::set<PixelPosition> expectedPositions{
		{ 1, 1 }, { 2, 1 }, { 3, 2 },
		{ 4, 2 }, { 5, 3 }, { 6, 3 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}

TEST(BresenhamLine, DrawsHighSlopeLine) {
	// A steep line must advance through every y coordinate exactly once.
	Framebuffer framebuffer{ 8, 8, background };
	drawLine(framebuffer, glm::ivec2{ 2, 1 }, glm::ivec2{ 4, 7 }, lineColor);

	const std::set<PixelPosition> expectedPositions{
		{ 2, 1 }, { 2, 2 }, { 3, 3 }, { 3, 4 },
		{ 3, 5 }, { 4, 6 }, { 4, 7 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}

TEST(BresenhamLine, HandlesVerticalLine) {
	// A vertical line is the high-slope boundary case where x never changes.
	Framebuffer framebuffer{ 8, 8, background };
	drawLine(framebuffer, glm::ivec2{ 3, 1 }, glm::ivec2{ 3, 5 }, lineColor);

	const std::set<PixelPosition> expectedPositions{
		{ 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}

TEST(BresenhamLine, IsIndependentOfEndpointOrder) {
	// Reversing endpoints should describe the same geometric set of pixels.
	Framebuffer framebuffer{ 8, 8, background };
	drawLine(framebuffer, glm::ivec2{ 6, 3 }, glm::ivec2{ 1, 1 }, lineColor);

	const std::set<PixelPosition> expectedPositions{
		{ 1, 1 }, { 2, 1 }, { 3, 2 },
		{ 4, 2 }, { 5, 3 }, { 6, 3 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}

TEST(BresenhamLine, IgnoresPixelsOutsideFramebuffer) {
	// Lines may cross the viewport boundary, but only visible pixels may be written.
	Framebuffer framebuffer{ 4, 4, background };
	drawLine(framebuffer, glm::ivec2{ -2, 1 }, glm::ivec2{ 3, 3 }, lineColor);

	const std::set<PixelPosition> expectedPositions{
		{ 0, 2 }, { 1, 2 }, { 2, 3 }, { 3, 3 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}

} // namespace
