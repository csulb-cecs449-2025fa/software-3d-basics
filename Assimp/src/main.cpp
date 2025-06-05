/**
* Renders a wireframe bunny loaded from an OBJ file using the Assimp C++ library.
* The bunny is drawn entirely in software, using SFML to access the framebuffer.
* The bunny is transformed from local space to world space (position, orientation, scale),
* then to clip space using a frustum for a camera at (0, 0, 0) looking down the negative Z axis.
*/
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <glm/ext.hpp>
#include <vector>
#include "triangles.h"
#define _USE_MATH_DEFINES // for M_PI
#include <math.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define LOG_FPS
struct Vertex3D {
	float x;
	float y;
	float z;
};
struct Frustum {
	float near;
	float far; 
	float right;
	float top;
};

const size_t FLOATS_PER_VERTEX = 3;
const size_t VERTICES_PER_FACE = 3;

// Reads the vertices and faces of an Assimp mesh, and uses them to initialize mesh structures
// compatible with the rest of our application.
void fromAssimpMesh(const aiMesh* mesh, std::vector<Vertex3D> &vertices,
	std::vector<uint32_t> &faces) {
	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		// Each "vertex" from Assimp has to be transformed into a Vertex3D in our application.
		vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
	}

	faces.reserve(mesh->mNumFaces * VERTICES_PER_FACE);
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		// We assume the faces are triangular, so we push three face indexes at a time into our faces list.
		faces.push_back(mesh->mFaces[i].mIndices[0]);
		faces.push_back(mesh->mFaces[i].mIndices[1]);
		faces.push_back(mesh->mFaces[i].mIndices[2]);
	}
}

// Loads an asset file supported by Assimp, extracts the first mesh in the file, and fills in the 
// given vertices and faces lists with its data.
void assimpLoad(const std::string& path, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& faces) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	// If the import failed, report it
	if (nullptr == scene) {
		std::cout << "ASSIMP ERROR" << importer.GetErrorString() << std::endl;
		exit(1);
	}
	else {
		fromAssimpMesh(scene->mMeshes[0], vertices, faces);
	}
}

Vertex3D localToWorld(
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const Vertex3D& vertex) {
	// Rotate, then scale, then translate.
	// When rotating, we first yaw, then pitch, then roll.
	// Yaw: rotating around the y-axis.
	float yawX = vertex.x * std::cos(orientation.y) + vertex.z * std::sin(orientation.y);
	float yawY = vertex.y;
	float yawZ = -vertex.x * std::sin(orientation.y) + vertex.z * std::cos(orientation.y);

	// Pitch: rotating around the x-axis, using (yawX, yawY yawZ) as the starting point.
	float pitchX = yawX;
	float pitchY = yawY * std::cos(orientation.x) - yawZ * std::sin(orientation.x);
	float pitchZ = yawY * std::sin(orientation.x) + yawZ * std::cos(orientation.x);

	// Roll: rotating around the z-axis, using (pitchX, pitchY, pitchZ) as the starting point.
	float rollX = pitchX * std::cos(orientation.z) - pitchY * std::sin(orientation.z);
	float rollY = pitchX * std::sin(orientation.z) + pitchY * std::cos(orientation.z);
	float rollZ = pitchZ;

	// Scale (rollX, rollY, rollZ) by the components of the scale vec3.
	float scaleX = rollX * scale.x; // fix these
	float scaleY = rollY * scale.y;
	float scaleZ = rollZ * scale.z;

	// Translate (scaleX, scaleY, scaleZ) by the components of the position vec3.
	float translateX = scaleX + position.x;
	float translateY = scaleY + position.y;
	float translateZ = scaleZ + position.z;

	return Vertex3D(translateX, translateY, translateZ);
}

// Transform from view coordinates to clip coordinates.
Vertex3D viewToClip(const Frustum& frustum, const Vertex3D& view) {
	float xp = view.x * -frustum.near / view.z;
	float yp = view.y * -frustum.near / view.z;
	float xClip = xp / frustum.right;
	float yClip = yp / frustum.top;
	return Vertex3D(xClip, yClip, 0);
}

// Linear interpolate from clip coordinates to screen coordinates.
sf::Vector2i clipToScreen(const sf::View& viewport, const Vertex3D& clip) {
	int32_t xs = static_cast<uint32_t>(viewport.getSize().x * (clip.x + 1) / 2.0);
	int32_t ys = static_cast<uint32_t>(viewport.getSize().y - viewport.getSize().y * (clip.y + 1) / 2.0);
	return sf::Vector2i(xs, ys);
}

void drawMesh(sf::RenderWindow& window, const Frustum& frustum,
	const sf::Vector3f& position, const sf::Vector3f& orientation, const sf::Vector3f& scale,
	const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& faces, sf::Color color) {
	// Loop through the list of face indexes, 3 at a time.
	// Pull each vertex out of the vertices list.
	// Transform them from clip coordinates to screen coordinates.
	// Draw a triangle connecting them.
	for (size_t i = 0; i < faces.size(); i = i + 3) {
		auto& vertexA = vertices[faces[i]];
		auto& vertexB = vertices[faces[i + 1]];
		auto& vertexC = vertices[faces[i + 2]];

		auto worldA = localToWorld(position, orientation, scale, vertexA);
		auto worldB = localToWorld(position, orientation, scale, vertexB);
		auto worldC = localToWorld(position, orientation, scale, vertexC);

		auto clipA = viewToClip(frustum, worldA);
		auto clipB = viewToClip(frustum, worldB);
		auto clipC = viewToClip(frustum, worldC);

		auto viewport = window.getView();
		auto screenA = clipToScreen(viewport, clipA);
		auto screenB = clipToScreen(viewport, clipB);
		auto screenC = clipToScreen(viewport, clipC);


		drawTriangle(window, 
			sf::Vector2i(screenA.x, screenA.y),
			sf::Vector2i(screenB.x, screenB.y),
			sf::Vector2i(screenC.x, screenC.y),
			color
		);
	}
}


int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	sf::Clock c;

	std::vector<Vertex3D> bunnyVertices;
	std::vector<uint32_t> bunnyFaces;
	assimpLoad("models/bunny.obj", bunnyVertices, bunnyFaces);

	sf::Vector3f bunnyPosition = sf::Vector3f(0, -1, -2.5);
	sf::Vector3f bunnyOrientation = sf::Vector3f(0, 0, 0);
	sf::Vector3f bunnyScale = sf::Vector3f(9, 9, 9);

	// Construct the frustum. Start with parameters near, far, fovy, and aspect ratio
	// to compute right and top.
	float fovy = 60; // This is a fairly narrow field of vision, for a screen that doesn't match
					 // the exact ratio of human vision.
	float ratio = static_cast<float>(window.getSize().x) / (window.getSize().y);
	float near = 0.1f;
	float far = 100.0f;
	float t = near * tan((fovy * M_PI / 180.0f) / 2);
	float r = t * ratio;
	Frustum frustum = Frustum(near, far, r, t);

	auto last = c.getElapsedTime();
	while (window.isOpen()) {
		// Check for events.
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}
		
#ifdef LOG_FPS
		// FPS calculation.
		auto now = c.getElapsedTime();
		auto diff = now - last;
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
#endif

		// Rotate the bunny by incrementing the orientation. This is a "yaw" around the y axis.
		bunnyPosition.z += 0.001f;

		// Render the scene.
		window.clear();
		drawMesh(window, frustum, bunnyPosition, bunnyOrientation, bunnyScale, bunnyVertices, bunnyFaces, sf::Color::White);
		window.display();
	}

	return 0;
}

// Why don't we see the cube in 3D? 
