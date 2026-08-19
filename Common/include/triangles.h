#pragma once

#include <glm/vec2.hpp>

#include "framebuffer.h"

void drawTriangle(
	Framebuffer& framebuffer,
	glm::ivec2 a,
	glm::ivec2 b,
	glm::ivec2 c,
	Pixel color);

