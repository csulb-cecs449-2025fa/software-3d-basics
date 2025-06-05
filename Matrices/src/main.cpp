/**
* Renders a wireframe bunny loaded from an OBJ file using the Assimp C++ library.
* The bunny is drawn entirely in software, using SFML to access the framebuffer.
* The bunny is transformed from local space to world space (position, orientation, scale),
* then to clip space using a frustum for a camera at (0, 0, 0) looking down the negative Z axis.
*/
#include <SFML/Graphics.hpp>
#include <iostream>
#include <glm/ext.hpp>
#include <vector>
#include <numbers>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "triangles.h"
#include "Mesh.h"


#define LOG_FPS

struct Frustum {
	float near;
	float far;
	float left;
	float right;
	float bottom;
	float top;
};

const size_t FLOATS_PER_VERTEX{ 3 };
const size_t VERTICES_PER_FACE{ 3 };

// Reads the vertices and faces of an Assimp mesh, and uses them to initialize mesh structures
// compatible with the rest of our application.
void fromAssimpMesh(const aiMesh* mesh, std::vector<Vertex3D>& vertices,
	std::vector<uint32_t>& faces) {
	for (size_t i{ 0 }; i < mesh->mNumVertices; ++i) {
		// Each "vertex" from Assimp has to be transformed into a Vertex3D in our application.
		vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
	}

	faces.reserve(mesh->mNumFaces * VERTICES_PER_FACE);
	for (size_t i{ 0 }; i < mesh->mNumFaces; ++i) {
		// We assume the faces are triangular, so we push three face indexes at a time into our faces list.
		faces.push_back(mesh->mFaces[i].mIndices[0]);
		faces.push_back(mesh->mFaces[i].mIndices[1]);
		faces.push_back(mesh->mFaces[i].mIndices[2]);
	}
}

// Loads an asset file supported by Assimp, extracts the first mesh in the file, and fills in the 
// given vertices and faces lists with its data.
void assimpLoad(const std::string& path, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& faces) {
	Assimp::Importer importer{};

	const aiScene* scene{ importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality) };

	// If the import failed, report it
	if (nullptr == scene) {
		std::cout << "ASSIMP ERROR" << importer.GetErrorString() << std::endl;
		exit(1);
	}
	else {
		fromAssimpMesh(scene->mMeshes[0], vertices, faces);
	}
}

glm::mat4 buildModelMatrix(const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale) {
	glm::mat4 model = glm::mat4(1);

	return model;
}

// Linear interpolate from clip coordinates to screen coordinates.
sf::Vector2i clipToScreen(const sf::View& viewport, const Vertex3D& clip) {
	int32_t xs{ static_cast<int32_t>(viewport.getSize().x * (clip.x + 1) / 2.0) };
	int32_t ys{ static_cast<int32_t>(viewport.getSize().y - viewport.getSize().y * (clip.y + 1) / 2.0) };
	return sf::Vector2i{ xs, ys };
}

void drawMesh(sf::RenderWindow& window,
	const glm::mat4& modelMatrix,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const std::vector<Vertex3D>& vertices,
	const std::vector<uint32_t>& faces,
	sf::Color color) {

	// TODO: first, construct a new 4x4 "MVP" matrix, by multiplying the
	// model, view, and projection matrices as shown in lecture. The order matters!!!
	glm::mat4 mvp{};

	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& localA{ vertices[faces[i]] };
		auto& localB{ vertices[faces[i + 1]] };
		auto& localC{ vertices[faces[i + 2]] };

		// TODO: transform vertexA, vertexB, and vertexC to clip space,
		// by using the MVP matrix you constructed.
		auto clipA{ localA };
		auto clipB{ localB };
		auto clipC{ localC };

		auto& viewport{ window.getView() };
		auto screenA{ clipToScreen(viewport, clipA) };
		auto screenB{ clipToScreen(viewport, clipB) };
		auto screenC{ clipToScreen(viewport, clipC) };

		drawTriangle(window,
			sf::Vector2i{ screenA.x, screenA.y },
			sf::Vector2i{ screenB.x, screenB.y },
			sf::Vector2i{ screenC.x, screenC.y },
			color
		);
	}
}


int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	sf::Clock c;

	std::vector<Vertex3D> bunnyVertices{};
	std::vector<uint32_t> bunnyFaces{};
	assimpLoad("models/bunny.obj", bunnyVertices, bunnyFaces);

	glm::vec3 bunnyPosition{ glm::vec3(0, -1, -2.5) };
	glm::vec3 bunnyOrientation{ glm::vec3(0, 0, 0) };
	glm::vec3 bunnyScale{ glm::vec3(9, 9, 9) };

	float fovy{ 60.0f };
	float ratio{ static_cast<float>(window.getSize().x) / (window.getSize().y) };
	float near{ 0.1f };
	float far{ 100.0f };
	float t{ static_cast<float>(near * tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2)) };
	float b{ -t };
	float r{ t * ratio };
	float l{ -r };

	auto last{ c.getElapsedTime() };
	while (window.isOpen()) {
		// Check for events.
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

#ifdef LOG_FPS
		// FPS calculation.
		auto now{ c.getElapsedTime() };
		auto diff{ now - last };
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif

		// Rotate the bunny by incrementing the orientation. This is a "yaw" around the y axis.
		bunnyOrientation.y += 0.001f;

		glm::mat4 bunnyModelMatrix{ buildModelMatrix(bunnyPosition, bunnyOrientation, bunnyScale) };
		glm::mat4 viewMatrix{ glm::mat4(1) };
		glm::mat4 projectionMatrix{ glm::mat4(1) };

		// Render the scene.
		window.clear();
		drawMesh(window, bunnyModelMatrix, viewMatrix, projectionMatrix, bunnyVertices, bunnyFaces, sf::Color::White);
		window.display();
	}

	return 0;
}
