#include <SFML/Graphics.hpp>

#include <cstdint>
#include <optional>
#include <vector>

#include "triangles.h"

// Clip coordinates are normalized device coordinates: the center is (0, 0),
// the lower-left is (-1, -1), and the upper-right is (1, 1).
struct Vertex2D {
	float x;
	float y;
};

glm::ivec2 clipToScreen(const Framebuffer& framebuffer, const Vertex2D& clip) {
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
	const std::vector<Vertex2D>& vertices,
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
		"Clip Space"
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

	const std::vector<Vertex2D> houseVertices{
		{ -0.5f, 0.0f },
		{ 0.5f, 0.0f },
		{ -0.5f, -0.5f },
		{ 0.5f, -0.5f },
		{ 0.0f, 0.5f }
	};
	const std::vector<uint32_t> houseFaces{
		0, 1, 2, 1, 3, 2, 0, 4, 1
	};

	while (window.isOpen()) {
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		framebuffer.clear(Pixel{});
		drawMesh(framebuffer, houseVertices, houseFaces);

		framebufferTexture.update(
			reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
		window.clear(sf::Color::Black);
		window.draw(framebufferSprite);
		window.display();
	}

	return 0;
}
