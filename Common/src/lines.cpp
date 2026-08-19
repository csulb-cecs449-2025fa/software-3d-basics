#include "lines.h"

#include <cstdlib>

void drawPixel(Framebuffer& framebuffer, glm::ivec2 position, Pixel color) {
	framebuffer.setPixel(position, color);
}

void drawLine(
	Framebuffer& framebuffer,
	glm::ivec2 start,
	glm::ivec2 end,
	Pixel color) {
	int x{ start.x };
	int y{ start.y };
	const int dx{ std::abs(end.x - start.x) };
	const int sx{ start.x < end.x ? 1 : -1 };
	const int dy{ -std::abs(end.y - start.y) };
	const int sy{ start.y < end.y ? 1 : -1 };
	int error{ dx + dy };

	while (true) {
		drawPixel(framebuffer, glm::ivec2{ x, y }, color);

		if (x == end.x && y == end.y) {
			break;
		}

		const int doubledError{ 2 * error };
		if (doubledError >= dy) {
			error += dy;
			x += sx;
		}
		if (doubledError <= dx) {
			error += dx;
			y += sy;
		}
	}
}

