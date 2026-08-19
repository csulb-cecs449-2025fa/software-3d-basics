#include <SFML/Graphics.hpp>
#include <cstdint>
#include <glm/vec3.hpp>
#include <vector>
#include <numbers>

#include "triangles.h"

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
Vertex3D worldToView(const glm::vec3& cameraPosition, const glm::vec3& cameraOrientation,
	const Vertex3D& vertex) {
	// Assumption: the camera is put in the scene first by orienting it (yaw, pitch, roll), 
	// then translating to its position. Instead of positioning the camera in the world, we 
	// will invert the camera's transformation and apply it to each vertex.

	// Reverse the position and orientation value.
	glm::vec3 cOrientation{ -cameraOrientation };

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
glm::ivec2 clipToScreen(const Framebuffer& framebuffer, const Vertex3D& clip) {
	int32_t xs{ static_cast<int32_t>(framebuffer.width() * (clip.x + 1) / 2.0) };
	int32_t ys{ static_cast<int32_t>(framebuffer.height() - framebuffer.height() * (clip.y + 1) / 2.0) };
	return glm::ivec2{ xs, ys };
}

void drawMesh(Framebuffer& framebuffer, const Frustum& frustum,
	const glm::vec3 & cameraPosition, const glm::vec3 & cameraOrientation,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces, Pixel color) {
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

		auto screenA{ clipToScreen(framebuffer, clipA) };
		auto screenB{ clipToScreen(framebuffer, clipB) };
		auto screenC{ clipToScreen(framebuffer, clipC) };

		drawTriangle(framebuffer,
			glm::ivec2{ screenA.x, screenA.y },
			glm::ivec2{ screenB.x, screenB.y },
			glm::ivec2{ screenC.x, screenC.y },
			color
		);
	}
}

int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	const auto windowSize{ window.getSize() };
	Framebuffer framebuffer{
		static_cast<int>(windowSize.x),
		static_cast<int>(windowSize.y),
		Pixel{}
	};
	sf::Texture framebufferTexture{ windowSize };
	framebufferTexture.setSmooth(false);
	sf::Sprite framebufferSprite{ framebufferTexture };

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
	float ratio{ static_cast<float>(windowSize.x) / (windowSize.y) };
	float near{ 0.1f };
	float far{ 100.0f };
	float t{ static_cast<float>(near * tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2)) };
	float b{ -t };
	float r{ t * ratio };
	float l{ -r };
	Frustum frustum{ near, far, l, r, b, t };

	// Position the camera.
	glm::vec3 cameraPosition{ 0, 0, 3 };
	glm::vec3 cameraOrientation{ 0, 0, 0 };

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

		// Render the scene.
		framebuffer.clear(Pixel{});
		drawMesh(framebuffer, frustum, cameraPosition, cameraOrientation,
			cubeVertices, cubeFaces, Pixel{ 255, 0, 0, 255 });
		framebufferTexture.update(
			reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
		window.clear(sf::Color::Black);
		window.draw(framebufferSprite);
		window.display();
	}

	return 0;
}

// Why don't we see the cube in 3D? 
