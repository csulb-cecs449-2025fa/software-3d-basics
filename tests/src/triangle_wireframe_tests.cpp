#include <gtest/gtest.h>

#include <set>

#include "raster_test_helpers.h"
#include "triangles.h"

TEST(TriangleWireframe, DrawsAllThreeEdges) {
	// A wireframe triangle should be the union of its three Bresenham edges.
	const Pixel background{};
	const Pixel edgeColor{ 255, 255, 255, 255 };
	Framebuffer framebuffer{ 8, 8, background };

	drawTriangle(
		framebuffer,
		glm::ivec2{ 1, 1 },
		glm::ivec2{ 5, 1 },
		glm::ivec2{ 3, 4 },
		edgeColor);

	const std::set<PixelPosition> expectedPositions{
		{ 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 1 },
		{ 2, 2 }, { 4, 2 }, { 2, 3 }, { 4, 3 }, { 3, 4 }
	};
	EXPECT_EQ(nonBackgroundPositions(framebuffer, background), expectedPositions);
}
