#include "triangles.h"
#include "lines.h"

void drawTriangle(sf::RenderWindow& window, glm::ivec2 a, glm::ivec2 b,
	glm::ivec2 c, sf::Color color) {
	drawLine(window, a, b, color);
	drawLine(window, a, c, color);
	drawLine(window, b, c, color);
}