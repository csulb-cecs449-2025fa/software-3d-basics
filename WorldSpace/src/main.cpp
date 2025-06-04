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
struct Frustum {
	float near;
	float far; 
	float right;
	float top;
};

Vertex3D localToWorld(
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const Vertex3D& vertex) {
	// Rotate, then scale, then translate.
	// When rotating, we first yaw, then pitch, then roll.
	// Yaw: rotating around the y-axis.
	float yawX = vertex.x * std::cos(orientation.y) + vertex.z * std::sin(orientation.y);
	float yawY = vertex.y;
	float yawZ = -vertex.x * std::sin(orientation.y) + vertex.z * std::cos(orientation.y);

	// Pitch: rotating around the x-axis, using (yawX, yawY yawZ) as the starting point.
	float pitchX = yawX;
	float pitchY = yawY * std::cos(orientation.x) - yawZ * std::sin(orientation.x);
	float pitchZ = yawY * std::sin(orientation.x) + yawZ * std::cos(orientation.x);

	// Roll: rotating around the z-axis, using (pitchX, pitchY, pitchZ) as the starting point.
	float rollX = pitchX * std::cos(orientation.z) - pitchY * std::sin(orientation.z);
	float rollY = pitchX * std::sin(orientation.z) + pitchY * std::cos(orientation.z);
	float rollZ = pitchZ;

	// Scale (rollX, rollY, rollZ) by the components of the scale vec3.
	float scaleX = rollX * scale.x; // fix these
	float scaleY = rollY * scale.y;
	float scaleZ = rollZ * scale.z;

	// Translate (scaleX, scaleY, scaleZ) by the components of the position vec3.
	float translateX = scaleX + position.x;
	float translateY = scaleY + position.y;
	float translateZ = scaleZ + position.z;

	return Vertex3D(translateX, translateY, translateZ);
}

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
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces, sf::Color color) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& vertexA = vertices[faces[i]];
		auto& vertexB = vertices[faces[i + 1]];
		auto& vertexC = vertices[faces[i + 2]];

		auto worldA = localToWorld(position, orientation, scale, vertexA);
		auto worldB = localToWorld(position, orientation, scale, vertexB);
		auto worldC = localToWorld(position, orientation, scale, vertexC);

		auto clipA = viewToClip(frustum, worldA);
		auto clipB = viewToClip(frustum, worldB);
		auto clipC = viewToClip(frustum, worldC);

		auto viewport = window.getView();
		auto screenA = clipToScreen(viewport, clipA);
		auto screenB = clipToScreen(viewport, clipB);
		auto screenC = clipToScreen(viewport, clipC);

		drawTriangle(window, 
			sf::Vector2i(screenA.x, screenA.y),
			sf::Vector2i(screenB.x, screenB.y),
			sf::Vector2i(screenC.x, screenC.y),
			color
		);
	}
}

int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	sf::Clock c;
	auto last = c.getElapsedTime();

	// Define the vertices and faces of the mesh we're drawing.
	// These are now LOCAL SPACE COORDINATES. We will separately set the
	// mesh's world space position, orientation, and scale, remembering
	// that the camera is at (0, 0, 0) looking down the negative z axis.
	std::vector<Vertex3D> cubeVertices = {
		{ 0.5, 0.5, -0.5 },
		{ -0.5, 0.5, -0.5 },
		{ -0.5, -0.5, -0.5 },
		{ 0.5, -0.5, -0.5 },
		{ 0.5, 0.5, 0.5 },
		{ -0.5, 0.5, 0.5 },
		{ -0.5, -0.5, 0.5 },
		{ 0.5, -0.5, 0.5 }
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
	// Move "back" away from the camera.
	sf::Vector3f position1 = sf::Vector3f(-1.5, 0, -3);
	// Yaw 15 degrees, pitch 22.5 degrees.
	sf::Vector3f orientation1 = sf::Vector3f(M_PI / 12, M_PI / 8, 0);
	// 100% scale.
	sf::Vector3f scale1 = sf::Vector3f(1, 1, 1);

	sf::Vector3f position2 = sf::Vector3f(0, 0, -6);
	sf::Vector3f orientation2 = sf::Vector3f(0, 0, 0);
	sf::Vector3f scale2 = sf::Vector3f(2, 2, 2);

	sf::Vector3f position3 = sf::Vector3f(0.5, 0, -2);
	sf::Vector3f orientation3 = sf::Vector3f(0, 0, M_PI/12);
	sf::Vector3f scale3 = sf::Vector3f(1, 1, 1);


	// Construct the frustum. Start with parameters near, far, fovy, and aspect ratio
	// to compute right and top.
	float fovy = 60; // This is a fairly narrow field of vision, for a screen that doesn't match
					 // the exact ratio of human vision.
	float ratio = static_cast<float>(window.getSize().x) / (window.getSize().y);
	float near = 0.1;
	float far = 100.0;
	float t = near * tan((fovy * M_PI / 180.0) / 2);
	float r = t * ratio;
	Frustum frustum = Frustum(near, far, r, t);


	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}
		window.clear();

		
#ifdef LOG_FPS
		// FPS calculation.
		auto now = c.getElapsedTime();
		auto diff = now - last;
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif

		// Rotate the cube by incrementing the orientation. This is a "yaw" around the y axis.
		orientation1.y += 0.0001;

		// Render the scene.
		window.clear();
		drawMesh(window, frustum, position1, orientation1, scale1, cubeVertices, cubeFaces, sf::Color::Red);
		drawMesh(window, frustum, position2, orientation2, scale2, cubeVertices, cubeFaces, sf::Color::Green);
		drawMesh(window, frustum, position3, orientation3, scale3, cubeVertices, cubeFaces, sf::Color::Blue);
		window.display();
	}

	return 0;
}

// Why don't we see the cube in 3D? 
