#include <SFML/Graphics.hpp>
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
Vertex3D localToWorld(
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const Vertex3D& vertex) {
	// Rotate, then scale, then translate.
	// When rotating, we first yaw, then pitch, then roll.
	// Yaw: rotating around the y-axis.
	float yawX{ vertex.x * std::cos(orientation.y) + vertex.z * std::sin(orientation.y) };
	float yawY = vertex.y;
	float yawZ{ -vertex.x * std::sin(orientation.y) + vertex.z * std::cos(orientation.y) };

	// Pitch: rotating around the x-axis, using (yawX, yawY yawZ) as the starting point.
	float pitchX{ yawX };
	float pitchY{ yawY * std::cos(orientation.x) - yawZ * std::sin(orientation.x) };
	float pitchZ{ yawY * std::sin(orientation.x) + yawZ * std::cos(orientation.x) };

	// Roll: rotating around the z-axis, using (pitchX, pitchY, pitchZ) as the starting point.
	float rollX{ pitchX * std::cos(orientation.z) - pitchY * std::sin(orientation.z) };
	float rollY{ pitchX * std::sin(orientation.z) + pitchY * std::cos(orientation.z) };
	float rollZ{ pitchZ };

	// Scale (rollX, rollY, rollZ) by the components of the scale vec3.
	float scaleX{ rollX * scale.x }; // fix thes}e
	float scaleY{ rollY * scale.y };
	float scaleZ{ rollZ * scale.z };

	// Translate (scaleX, scaleY, scaleZ) by the components of the position vec3.
	float translateX{ scaleX + position.x };
	float translateY{ scaleY + position.y };
	float translateZ{ scaleZ + position.z };

	return Vertex3D{ translateX, translateY, translateZ };
}

// Transforms from local coordinates to world coordinates.
Vertex3D worldToView(const sf::Vector3f& cameraPosition, const sf::Vector3f& cameraOrientation, const Vertex3D& vertex) {
	// Assumption: the camera is put in the scene first by orienting it (yaw, pitch, roll), 
	// then translating to its position. Instead of positioning the camera in the world, we 
	// will invert the camera's transformation and apply it to each vertex.

	// Reverse the position and orientation value.
	sf::Vector3f cOrientation{ -cameraOrientation };

	float translateX{ vertex.x - cameraPosition.x };
	float translateY{ vertex.y - cameraPosition.y };
	float translateZ{ vertex.z - cameraPosition.z };

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

	return Vertex3D{ yawX, yawY, yawZ };
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
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces, sf::Color color) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i{ 0 }; i < faces.size(); i = i + 3) {
		auto& localA{ vertices[faces[i]] };
		auto& localB{ vertices[faces[i + 1]] };
		auto& localC{ vertices[faces[i + 2]] };

		auto worldA{ localToWorld(position, orientation, scale, localA) };
		auto worldB{ localToWorld(position, orientation, scale, localB) };
		auto worldC{ localToWorld(position, orientation, scale, localC) };

		auto viewA{ worldToView(cameraPosition, cameraOrientation, worldA) };
		auto viewB{ worldToView(cameraPosition, cameraOrientation, worldB) };
		auto viewC{ worldToView(cameraPosition, cameraOrientation, worldC) };

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
	// These are now LOCAL SPACE COORDINATES. We will separately set the
	// mesh's world space position, orientation, and scale, remembering
	// that the camera is at (0, 0, 0) looking down the negative z axis.
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

	// Move "back" away from the camera.
	sf::Vector3f position1{ sf::Vector3f{-1.5, 0, 0} };
	// Yaw 15 degrees, pitch 22.5 degrees.
	sf::Vector3f orientation1{ sf::Vector3f{std::numbers::pi_v<float> / 12, std::numbers::pi_v<float> / 8, 0} };
	// 100% scale.
	sf::Vector3f scale1{ sf::Vector3f{1, 1, 1} };

	sf::Vector3f position2{ sf::Vector3f{0, 0, -3} };
	sf::Vector3f orientation2{ sf::Vector3f{0, 0, 0} };
	sf::Vector3f scale2{ sf::Vector3f{2, 2, 2} };

	sf::Vector3f position3{ sf::Vector3f{0.5, 0, 1} };
	sf::Vector3f orientation3{ sf::Vector3f{0, 0, std::numbers::pi_v<float> / 12} };
	sf::Vector3f scale3{ sf::Vector3f{1, 1, 1} };


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
			cameraPosition = { 1.5f, 0, 2.6f };
			cameraOrientation = { 0, std::numbers::pi_v<float> / 6, 0 };
		}

#ifdef LOG_FPS
		// FPS calculation.
		auto now{ c.getElapsedTime() };
		auto diff{ now - last };
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif

		// Rotate the cube by incrementing the orientation. This is a "yaw" around the y axis.
		orientation1.y += 0.0001f;

		// Render the scene.
		window.clear();
		drawMesh(window, frustum, cameraPosition, cameraOrientation, position1, orientation1, scale1, cubeVertices, cubeFaces, sf::Color::Red);
		drawMesh(window, frustum, cameraPosition, cameraOrientation, position2, orientation2, scale2, cubeVertices, cubeFaces, sf::Color::Green);
		drawMesh(window, frustum, cameraPosition, cameraOrientation, position3, orientation3, scale3, cubeVertices, cubeFaces, sf::Color::Blue);
		window.display();
	}

	return 0;
}

// Why don't we see the cube in 3D? 
