#include <SFML/Graphics.hpp>
#include <iostream>

#include <memory>
#include <glm/ext.hpp>
#include <vector>
#include "triangles.h"
#define _USE_MATH_DEFINES // for M_PI
#include <math.h>

#define LOG_FPS
struct Vertex3D {
	float x;
	float y;
	float z;
};

// Parameters to define the viewing frustum.
struct Frustum {
	float near;
	float far;
	float left;
	float right;
	float bottom;
	float top;
};

// Transform from view coordinates to clip coordinates.
Vertex3D viewToClip(const Frustum& frustum, const Vertex3D& view) {
	float xp = view.x * -frustum.near / view.z;
	float yp = view.y * -frustum.near / view.z;
	float xClip = xp / frustum.right;
	float yClip = yp / frustum.top;
	return Vertex3D(xClip, yClip, 0);
}

// Linear interpolate from clip coordinates to screen coordinates.
sf::Vector2i clipToScreen(const sf::View& viewport, const Vertex3D& clip) {
	int32_t xs = static_cast<uint32_t>(viewport.getSize().x * (clip.x + 1) / 2.0);
	int32_t ys = static_cast<uint32_t>(viewport.getSize().y - viewport.getSize().y * (clip.y + 1) / 2.0);
	return sf::Vector2i(xs, ys);
}

void drawMesh(sf::RenderWindow& window, const Frustum& frustum,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& vertexA = vertices[faces[i]];
		auto& vertexB = vertices[faces[i + 1]];
		auto& vertexC = vertices[faces[i + 2]];

		auto clipA = viewToClip(frustum, vertexA);
		auto clipB = viewToClip(frustum, vertexB);
		auto clipC = viewToClip(frustum, vertexC);

		auto viewport = window.getView();
		auto screenA = clipToScreen(viewport, clipA);
		auto screenB = clipToScreen(viewport, clipB);
		auto screenC = clipToScreen(viewport, clipC);

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
	// These are now VIEW COORDINATES, so we need to "back away" from the camera,
	// which is at (0, 0, 0).
	std::vector<Vertex3D> cubeVertices = {
		{ 0.5, 0.5, -3.5 },
		{ -0.5, 0.5, -3.5 },
		{ -0.5, -0.5, -3.5 },
		{ 0.5, -0.5, -3.5 },
		{ 0.5, 0.5, -2.5 },
		{ -0.5, 0.5, -2.5 },
		{ -0.5, -0.5, -2.5 },
		{ 0.5, -0.5, -2.5 }
	};
	std::vector<uint32_t> cubeFaces = {
		0, 1, 2,
		0, 2, 3,
		4, 0, 3,
		4, 3, 7,
		5, 4, 7,
		5, 7, 6,
		1, 5, 6,
		1, 6, 2,
		4, 5, 1,
		4, 1, 0,
		2, 6, 7,
		2, 7, 3
	};

	// Construct the frustum. Start with parameters near, far, fovy, and aspect ratio
	// to compute left, right, bottom, and top.
	float fovy = 60;
	float ratio = static_cast<float>(window.getSize().x) / (window.getSize().y);
	float near = 0.1;
	float far = 100.0;
	float t = near * tan((fovy * M_PI / 180.0) / 2);
	float b = -t;
	float r = t * ratio;
	float l = -r;
	Frustum frustum = Frustum(near, far, l, r, b, t);

	auto last = c.getElapsedTime();
	while (window.isOpen()) {
		// Check for events.
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
		drawMesh(window, frustum, cubeVertices, cubeFaces);
		window.display();
	}

	return 0;
}

// What if we want to position the cube somewhere else, or animate it?
