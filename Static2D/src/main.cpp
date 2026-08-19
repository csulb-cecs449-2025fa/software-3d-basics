#include <SFML/Graphics.hpp>

#include <cstdint>
#include <optional>
#include <vector>

#include "triangles.h"

struct Vertex2D {
	int32_t x;
	int32_t y;
};

void drawMesh(
	Framebuffer& framebuffer,
	const std::vector<Vertex2D>& vertices,
	const std::vector<uint32_t>& faces) {
	for (size_t i{ 0 }; i < faces.size(); i += 3) {
		const auto& vertexA{ vertices[faces[i]] };
		const auto& vertexB{ vertices[faces[i + 1]] };
		const auto& vertexC{ vertices[faces[i + 2]] };

		drawTriangle(
			framebuffer,
			glm::ivec2{ vertexA.x, vertexA.y },
			glm::ivec2{ vertexB.x, vertexB.y },
			glm::ivec2{ vertexC.x, vertexC.y },
			Pixel{ 255, 255, 255, 255 });
	}
}

int main() {
	sf::RenderWindow window{
		sf::VideoMode::getFullscreenModes().at(0),
		"Static 2D"
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
		{ 300, 300 },
		{ 600, 300 },
		{ 300, 500 },
		{ 600, 500 },
		{ 450, 150 }
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
