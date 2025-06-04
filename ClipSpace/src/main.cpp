#include <SFML/Graphics.hpp>
#include <iostream>

#include <memory>
#include <glm/ext.hpp>
#include <vector>
#include "triangles.h"

#define LOG_FPS
struct Vertex2D {
	float x;
	float y;
};

// Linear interpolate from clip coordinates to screen coordinates.
sf::Vector2i clipToScreen(const sf::View& viewport, const Vertex2D& clip) {
	int32_t xs = static_cast<uint32_t>(viewport.getSize().x * (clip.x + 1) / 2.0);
	int32_t ys = static_cast<uint32_t>(viewport.getSize().y - viewport.getSize().y * (clip.y + 1) / 2.0);
	return sf::Vector2i(xs, ys);
}

void drawMesh(sf::RenderWindow& window, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& faces) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& vertexA = vertices[faces[i]];
		auto& vertexB = vertices[faces[i + 1]];
		auto& vertexC = vertices[faces[i + 2]];

		auto viewport = window.getView();
		auto screenA = clipToScreen(viewport, vertexA);
		auto screenB = clipToScreen(viewport, vertexB);
		auto screenC = clipToScreen(viewport, vertexC);

		drawTriangle(window,
			sf::Vector2i(screenA.x, screenA.y),
			sf::Vector2i(screenB.x, screenB.y),
			sf::Vector2i(screenC.x, screenC.y),
			sf::Color::White
		);
	}
}


int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	sf::Clock c;


	// Define the vertices and faces of the mesh we're drawing.
	std::vector<Vertex2D> houseVertices = {
		{-0.5, 0.0},
		{0.5, 0.0},
		{-0.5, -0.5},
		{0.5, -0.5},
		{0.0, 0.5}
	};
	std::vector<uint32_t> houseFaces = {
		0, 1, 2, 1, 3, 2, 0, 4, 1
	};

	auto last = c.getElapsedTime();

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

#ifdef LOG_FPS
		// FPS calculation.
		auto now = c.getElapsedTime();
		auto diff = now - last;
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif
		// Render the scene.
		window.clear();
		drawMesh(window, houseVertices, houseFaces);
		window.display();
	}

	return 0;
}

// But this still isn't 3D, right? We're just connecting triangles.
// So let's try giving vertices a Z coordinate.
