#include <SFML/Graphics.hpp>

#include <cstdint>
#include <optional>
#include <vector>

#include "triangles.h"

struct Vertex3D {
	float x;
	float y;
	float z;
};

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
	const std::vector<Vertex3D>& vertices,
	const std::vector<uint32_t>& faces) {
	for (size_t i{ 0 }; i < faces.size(); i += 3) {
		const auto& vertexA{ vertices[faces[i]] };
		const auto& vertexB{ vertices[faces[i + 1]] };
		const auto& vertexC{ vertices[faces[i + 2]] };

		const auto screenA{ clipToScreen(framebuffer, vertexA) };
		const auto screenB{ clipToScreen(framebuffer, vertexB) };
		const auto screenC{ clipToScreen(framebuffer, vertexC) };

		drawTriangle(framebuffer, screenA, screenB, screenC, Pixel{ 255, 255, 255, 255 });
	}
}

int main() {
	sf::RenderWindow window{
		sf::VideoMode::getFullscreenModes().at(0),
		"Vertex 3D"
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
		{ 0.5f, 0.5f, -0.5f },
		{ -0.5f, 0.5f, -0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{ 0.5f, -0.5f, -0.5f },
		{ 0.5f, 0.5f, 0.5f },
		{ -0.5f, 0.5f, 0.5f },
		{ -0.5f, -0.5f, 0.5f },
		{ 0.5f, -0.5f, 0.5f }
	};
	const std::vector<uint32_t> cubeFaces{
		0, 1, 2, 0, 2, 3,
		4, 0, 3, 4, 3, 7,
		5, 4, 7, 5, 7, 6,
		1, 5, 6, 1, 6, 2,
		4, 5, 1, 4, 1, 0,
		2, 6, 7, 2, 7, 3
	};

	while (window.isOpen()) {
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		framebuffer.clear(Pixel{});
		drawMesh(framebuffer, cubeVertices, cubeFaces);

		framebufferTexture.update(
			reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
		window.clear(sf::Color::Black);
		window.draw(framebufferSprite);
		window.display();
	}

	return 0;
}
