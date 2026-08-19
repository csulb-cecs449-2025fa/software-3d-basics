#include "framebuffer.h"

#include <algorithm>
#include <stdexcept>

Framebuffer::Framebuffer(int width, int height, Pixel clearColor)
	: m_width{ width },
	  m_height{ height },
	  m_pixels{} {
	if (width <= 0 || height <= 0) {
		throw std::invalid_argument("Framebuffer dimensions must be positive");
	}

	m_pixels.resize(
		static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height),
		clearColor);
}

int Framebuffer::width() const noexcept {
	return m_width;
}

int Framebuffer::height() const noexcept {
	return m_height;
}

bool Framebuffer::contains(glm::ivec2 position) const noexcept {
	return position.x >= 0 && position.x < m_width
		&& position.y >= 0 && position.y < m_height;
}

void Framebuffer::clear(Pixel color) {
	std::fill(m_pixels.begin(), m_pixels.end(), color);
}

bool Framebuffer::setPixel(glm::ivec2 position, Pixel color) noexcept {
	if (!contains(position)) {
		return false;
	}

	m_pixels[index(position)] = color;
	return true;
}

Pixel Framebuffer::getPixel(glm::ivec2 position) const {
	if (!contains(position)) {
		throw std::out_of_range("Framebuffer pixel position is out of bounds");
	}

	return m_pixels[index(position)];
}

std::vector<Pixel>& Framebuffer::data() noexcept {
	return m_pixels;
}

const std::vector<Pixel>& Framebuffer::data() const noexcept {
	return m_pixels;
}

std::size_t Framebuffer::index(glm::ivec2 position) const noexcept {
	return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(m_width)
		+ static_cast<std::size_t>(position.x);
}
