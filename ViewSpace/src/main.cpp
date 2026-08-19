#include <SFML/Graphics.hpp>

#include <cmath>
#include <cstdint>
#include <numbers>
#include <optional>
#include <vector>

#include "triangles.h"

struct Vertex3D {
	float x;
	float y;
	float z;
};

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
	const float xp{ view.x * -frustum.near / view.z };
	const float yp{ view.y * -frustum.near / view.z };
	const float xClip{ xp / frustum.right };
	const float yClip{ yp / frustum.top };
	return Vertex3D{ xClip, yClip, 0.0f };
}

glm::ivec2 clipToScreen(const Framebuffer& framebuffer, const Vertex3D& clip) {
	const int32_t xs{
		static_cast<int32_t>(framebuffer.width() * (clip.x + 1.0f) / 2.0f)
	};
	const int32_t ys{
		static_cast<int32_t>(
			framebuffer.height()
			- framebuffer.height() * (clip.y + 1.0f) / 2.0f)
	};
	return glm::ivec2{ xs, ys };
}

void drawMesh(
	Framebuffer& framebuffer,
	const Frustum& frustum,
	const std::vector<Vertex3D>& vertices,
	const std::vector<uint32_t>& faces) {
	for (size_t i{ 0 }; i < faces.size(); i += 3) {
		const auto& vertexA{ vertices[faces[i]] };
		const auto& vertexB{ vertices[faces[i + 1]] };
		const auto& vertexC{ vertices[faces[i + 2]] };

		const auto clipA{ viewToClip(frustum, vertexA) };
		const auto clipB{ viewToClip(frustum, vertexB) };
		const auto clipC{ viewToClip(frustum, vertexC) };

		const auto screenA{ clipToScreen(framebuffer, clipA) };
		const auto screenB{ clipToScreen(framebuffer, clipB) };
		const auto screenC{ clipToScreen(framebuffer, clipC) };

		drawTriangle(
			framebuffer,
			screenA,
			screenB,
			screenC,
			Pixel{ 255, 255, 255, 255 });
	}
}

int main() {
	sf::RenderWindow window{
		sf::VideoMode::getFullscreenModes().at(0),
		"View Space"
	};
	const auto windowSize{ window.getSize() };
	Framebuffer framebuffer{
		static_cast<int>(windowSize.x),
		static_cast<int>(windowSize.y),
		Pixel{}
	};
	sf::Texture framebufferTexture{ windowSize };
	framebufferTexture.setSmooth(false);
	sf::Sprite framebufferSprite{ framebufferTexture };

	const std::vector<Vertex3D> cubeVertices{
		{ 0.5f, 0.5f, -3.5f },
		{ -0.5f, 0.5f, -3.5f },
		{ -0.5f, -0.5f, -3.5f },
		{ 0.5f, -0.5f, -3.5f },
		{ 0.5f, 0.5f, -2.5f },
		{ -0.5f, 0.5f, -2.5f },
		{ -0.5f, -0.5f, -2.5f },
		{ 0.5f, -0.5f, -2.5f }
	};
	const std::vector<uint32_t> cubeFaces{
		0, 1, 2, 0, 2, 3,
		4, 0, 3, 4, 3, 7,
		5, 4, 7, 5, 7, 6,
		1, 5, 6, 1, 6, 2,
		4, 5, 1, 4, 1, 0,
		2, 6, 7, 2, 7, 3
	};

	const float fovy{ 60.0f };
	const float ratio{
		static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y)
	};
	const float nearPlane{ 0.1f };
	const float farPlane{ 100.0f };
	const float top{
		nearPlane * std::tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2.0f)
	};
	const float bottom{ -top };
	const float right{ top * ratio };
	const float left{ -right };
	const Frustum frustum{
		nearPlane,
		farPlane,
		left,
		right,
		bottom,
		top
	};

	while (window.isOpen()) {
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		framebuffer.clear(Pixel{});
		drawMesh(framebuffer, frustum, cubeVertices, cubeFaces);

		framebufferTexture.update(
			reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
		window.clear(sf::Color::Black);
		window.draw(framebufferSprite);
		window.display();
	}

	return 0;
}
