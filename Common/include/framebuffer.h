#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

#include <glm/vec2.hpp>

struct Pixel {
	std::uint8_t r{ 0 };
	std::uint8_t g{ 0 };
	std::uint8_t b{ 0 };
	std::uint8_t a{ 255 };

	constexpr bool operator==(const Pixel&) const = default;
};

static_assert(sizeof(Pixel) == 4);

inline std::ostream& operator<<(std::ostream& stream, Pixel pixel) {
	return stream
		<< "Pixel{r=" << static_cast<unsigned int>(pixel.r)
		<< ", g=" << static_cast<unsigned int>(pixel.g)
		<< ", b=" << static_cast<unsigned int>(pixel.b)
		<< ", a=" << static_cast<unsigned int>(pixel.a)
		<< "}";
}

/**
 * @brief A CPU-side, row-major RGBA framebuffer.
 *
 * Coordinates use the SFML convention: (0, 0) is the upper-left corner,
 * x increases to the right, and y increases downward.
 */
class Framebuffer {
public:
	Framebuffer(int width, int height, Pixel clearColor = {});

	[[nodiscard]] int width() const noexcept;
	[[nodiscard]] int height() const noexcept;
	[[nodiscard]] bool contains(glm::ivec2 position) const noexcept;

	void clear(Pixel color);
	bool setPixel(glm::ivec2 position, Pixel color) noexcept;
	[[nodiscard]] Pixel getPixel(glm::ivec2 position) const;

	[[nodiscard]] std::vector<Pixel>& data() noexcept;
	[[nodiscard]] const std::vector<Pixel>& data() const noexcept;

private:
	[[nodiscard]] std::size_t index(glm::ivec2 position) const noexcept;

	int m_width;
	int m_height;
	std::vector<Pixel> m_pixels;
};
