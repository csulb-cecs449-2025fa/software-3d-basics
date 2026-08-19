#include "triangles.h"

#include "lines.h"

void drawTriangle(
	Framebuffer& framebuffer,
	glm::ivec2 a,
	glm::ivec2 b,
	glm::ivec2 c,
	Pixel color) {
	drawLine(framebuffer, a, b, color);
	drawLine(framebuffer, b, c, color);
	drawLine(framebuffer, c, a, color);
}

