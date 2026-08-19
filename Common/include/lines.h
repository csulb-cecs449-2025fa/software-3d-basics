#pragma once

#include <glm/vec2.hpp>

#include "framebuffer.h"

void drawPixel(Framebuffer& framebuffer, glm::ivec2 position, Pixel color);
void drawLine(Framebuffer& framebuffer, glm::ivec2 start, glm::ivec2 end, Pixel color);

