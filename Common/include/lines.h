#pragma once
#include <SFML/Graphics.hpp>
#include <glm/ext.hpp>
void drawPixel(sf::RenderWindow& window, glm::ivec2 position, sf::Color color);
void drawLine(sf::RenderWindow& window, glm::ivec2 start, glm::ivec2 end, sf::Color color);
