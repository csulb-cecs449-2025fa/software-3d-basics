#include "lines.h"
#include <SFML/Graphics.hpp>
#include <array>


void drawPixel(sf::RenderWindow& window, sf::Vector2i position, sf::Color color) {
	float pX{ static_cast<float>(position.x) };
	float pY{ static_cast<float>(position.y) };

	std::array<sf::Vertex, 1> pixel{
		sf::Vertex{
			sf::Vector2f{pX, pY},
			color
		}
	};
	window.draw(pixel.data(), 1, sf::PrimitiveType::Points);
}

// This version of drawLine uses SFML to more-efficiently draw the line, instead of using repeated
// calls to our drawPixel.
// SFML uses Bresenham's algorithm like us; but they avoid the overhead of multiple "draw a pixel"
// calls because they have low-level access to the framebuffer.
// You can replace this method with your Bresenham's algorithms from Homework 1; the demo will run
// slower, but it will more truly be *your* own work.
void drawLine(sf::RenderWindow& window, sf::Vector2i start, sf::Vector2i end, sf::Color color) {
	float sX{ static_cast<float>(start.x) };
	float sY{ static_cast<float>(start.y) };
	float eX{ static_cast<float>(end.x) };
	float eY{ static_cast<float>(end.y) };
	std::array<sf::Vertex, 2> points {
		sf::Vertex{sf::Vector2f{sX, sY}, color},
		sf::Vertex{sf::Vector2f{eX, eY}, color}
	};
	window.draw(points.data(), 2, sf::PrimitiveType::Lines);
}
