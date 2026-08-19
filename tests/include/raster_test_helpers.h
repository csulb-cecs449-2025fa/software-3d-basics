#pragma once

#include <set>
#include <utility>

#include "framebuffer.h"

using PixelPosition = std::pair<int, int>;

inline std::set<PixelPosition> nonBackgroundPositions(
	const Framebuffer& framebuffer,
	Pixel background) {
	std::set<PixelPosition> positions;

	for (int y{ 0 }; y < framebuffer.height(); ++y) {
		for (int x{ 0 }; x < framebuffer.width(); ++x) {
			if (framebuffer.getPixel(glm::ivec2{ x, y }) != background) {
				positions.emplace(x, y);
			}
		}
	}

	return positions;
}
