#include <SFML/Graphics.hpp>
#include <iostream>

#include <memory>
#include <glm/ext.hpp>
#include <vector>
#include "triangles.h"

#define LOG_FPS
struct Vertex2D {
	int32_t x;
	int32_t y;
};

void drawMesh(sf::RenderWindow& window, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& faces) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& vertexA = vertices[faces[i]];
		auto& vertexB = vertices[faces[i + 1]];
		auto& vertexC = vertices[faces[i + 2]];

		drawTriangle(window, 
			sf::Vector2i(vertexA.x, vertexA.y),
			sf::Vector2i(vertexB.x, vertexB.y),
			sf::Vector2i(vertexC.x, vertexC.y),
			sf::Color::White
		);
	}
}


int main() {
	sf::RenderWindow window{ sf::VideoMode{1200, 800}, "SFML Demo" };
	sf::Clock c;

	// Define the vertices and faces of the mesh we're drawing.
	std::vector<Vertex2D> houseVertices = {
		{300, 300},
		{600, 300},
		{300, 500},
		{600, 500},
		{450, 150}
	};
	std::vector<uint32_t> houseFaces = {
		0, 1, 2, 1, 3, 2, 0, 4, 1
	};


	auto last = c.getElapsedTime();
	while (window.isOpen()) {
		// Check for events.
		sf::Event ev;
		while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
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

// What if we wanted to position things with *relative* coordinates,
// instead of pixel coordinates?
// We introduce NORMALIZED DEVICE COORDINATES, aka Clip Coordinates.
// Middle of the screen is (0, 0). 
// Lower left is (-1, -1).
// Upper right is (1, 1).

// Where is (1, 0)?
// Where is (-0.5, 0.25)?
