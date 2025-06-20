﻿#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <numbers>

#include "triangles.h"

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

// Transforms from local coordinates to world coordinates.
Vertex3D worldToView(const sf::Vector3f& cameraPosition, const sf::Vector3f& cameraOrientation,
	const Vertex3D& vertex) {
	// Assumption: the camera is put in the scene first by orienting it (yaw, pitch, roll), 
	// then translating to its position. Instead of positioning the camera in the world, we 
	// will invert the camera's transformation and apply it to each vertex.

	// Reverse the position and orientation value.
	sf::Vector3f cOrientation{ -cameraOrientation };

	float translateX { vertex.x - cameraPosition.x };
	float translateY { vertex.y - cameraPosition.y };
	float translateZ { vertex.z - cameraPosition.z };

	float rollX{ translateX * std::cos(cOrientation.z) - translateY * std::sin(cOrientation.z) };
	float rollY{ translateX * std::sin(cOrientation.z) + translateY * std::cos(cOrientation.z) };
	float rollZ{ translateZ };

	// Pitch: rotating around the x-axis, using (rollX, rollY, rollZ) as the starting point.
	float pitchX{ rollX };
	float pitchY{ rollY * std::cos(cOrientation.x) - rollZ * std::sin(cOrientation.x) };
	float pitchZ{ rollY * std::sin(cOrientation.x) + rollZ * std::cos(cOrientation.x) };

	float yawX{ pitchX * std::cos(cOrientation.y) + pitchZ * std::sin(cOrientation.y) };
	float yawY{ vertex.y };
	float yawZ{ -pitchX * std::sin(cOrientation.y) + pitchZ * std::cos(cOrientation.y) };

	return Vertex3D{ yawX, yawY, yawZ};
}

// Transform from view coordinates to clip coordinates.
Vertex3D viewToClip(const Frustum& frustum, const Vertex3D& view) {
	float xp{ view.x * -frustum.near / view.z };
	float yp{ view.y * -frustum.near / view.z };
	float xClip{ xp / frustum.right };
	float yClip{ yp / frustum.top };
	return Vertex3D{ xClip, yClip, 0.0f };
}

// Linear interpolate from clip coordinates to screen coordinates.
sf::Vector2i clipToScreen(const sf::View& viewport, const Vertex3D& clip) {
	int32_t xs{ static_cast<int32_t>(viewport.getSize().x * (clip.x + 1) / 2.0) };
	int32_t ys{ static_cast<int32_t>(viewport.getSize().y - viewport.getSize().y * (clip.y + 1) / 2.0) };
	return sf::Vector2i{ xs, ys };
}

void drawMesh(sf::RenderWindow& window, const Frustum& frustum,
	const sf::Vector3f& cameraPosition, const sf::Vector3f& cameraOrientation,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces, sf::Color color) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from world -> view -> clip -> screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i{ 0 }; i < faces.size(); i = i + 3) {
		auto& vertexA{ vertices[faces[i]] };
		auto& vertexB{ vertices[faces[i + 1]] };
		auto& vertexC{ vertices[faces[i + 2]] };

		auto viewA{ worldToView(cameraPosition, cameraOrientation, vertexA) };
		auto viewB{ worldToView(cameraPosition, cameraOrientation, vertexB) };
		auto viewC{ worldToView(cameraPosition, cameraOrientation, vertexC) };

		auto clipA{ viewToClip(frustum, viewA) };
		auto clipB{ viewToClip(frustum, viewB) };
		auto clipC{ viewToClip(frustum, viewC) };

		auto& viewport{ window.getView() };
		auto screenA{ clipToScreen(viewport, clipA) };
		auto screenB{ clipToScreen(viewport, clipB) };
		auto screenC{ clipToScreen(viewport, clipC) };

		drawTriangle(window,
			sf::Vector2i{ screenA.x, screenA.y },
			sf::Vector2i{ screenB.x, screenB.y },
			sf::Vector2i{ screenC.x, screenC.y },
			color
		);
	}
}

int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	sf::Clock c;

	// Define the vertices and faces of the mesh we're drawing.
	// These are now WORLD SPACE COORDINATES, in the same virtual space where 
	// we will define the camera.
	std::vector<Vertex3D> cubeVertices{
		{ 0.5, 0.5, -0.5 },
		{ -0.5, 0.5, -0.5 },
		{ -0.5, -0.5, -0.5 },
		{ 0.5, -0.5, -0.5 },
		{ 0.5, 0.5, 0.5 },
		{ -0.5, 0.5, 0.5 },
		{ -0.5, -0.5, 0.5 },
		{ 0.5, -0.5, 0.5 }
	};
	std::vector<uint32_t> cubeFaces{
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
	// to compute right and top.
	float fovy{ 60.0f };
	float ratio{ static_cast<float>(window.getSize().x) / (window.getSize().y) };
	float near{ 0.1f };
	float far{ 100.0f };
	float t{ static_cast<float>(near * tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2)) };
	float b{ -t };
	float r{ t * ratio };
	float l{ -r };
	Frustum frustum{ near, far, l, r, b, t };

	// Position the camera.
	sf::Vector3f cameraPosition{ 0, 0, 3 };
	sf::Vector3f cameraOrientation{ 0, 0, 0 };


	auto last{ c.getElapsedTime() };
	while (window.isOpen()) {
		// Check for events.
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num1)) {
			cameraPosition = { 0, 0, 3 };
			cameraOrientation = { 0, 0, 0 };
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num2)) {
			cameraPosition = { 0, 0, 5 };
			cameraOrientation = { 0, 0, 0 };
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num3)) {
			cameraPosition = { 0, 0, 2 };
			cameraOrientation = { 0, 0, 0 };
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num4)) {
			cameraPosition = { 1.5f, 0, 2.6f};
			cameraOrientation = { 0, std::numbers::pi_v<float>/6, 0 };
		}

#ifdef LOG_FPS
		// FPS calculation.
		auto now{ c.getElapsedTime() };
		auto diff{ now - last };
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif

		// Render the scene.
		window.clear();
		drawMesh(window, frustum, cameraPosition, cameraOrientation, cubeVertices, cubeFaces, sf::Color::Red);
		window.display();
	}

	return 0;
}

// Why don't we see the cube in 3D? 
