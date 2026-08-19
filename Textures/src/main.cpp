/**
* Renders a wireframe bunny loaded from an OBJ file using the Assimp C++ library.
* The bunny is drawn entirely in software, using SFML to access the framebuffer.
* The bunny is transformed from local space to world space (position, orientation, scale),
* then to clip space using a frustum for a camera at (0, 0, 0) looking down the negative Z axis.
*/
#include "StbImage.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <glm/ext.hpp>
#include <vector>
#include <numbers>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "triangles.h"

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
		vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z,
			mesh->mTextureCoords[0][i].x, 1 - mesh->mTextureCoords[0][i].y});
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
	glm::mat4 model{ 1 };
	// use the scale, orientation, and postition to build a model matrix that transforms from local space to world space
	model = glm::translate(model, position);
	model = glm::rotate(model, orientation.x, glm::vec3{ 1, 0, 0 });
	model = glm::rotate(model, orientation.y, glm::vec3{ 0, 1, 0 });
	model = glm::rotate(model, orientation.z, glm::vec3{ 0, 0, 1 });
	model = glm::scale(model, scale);

	return model;
}

// Linear interpolate from clip coordinates to screen coordinates.
glm::vec2 clipToScreen(const Framebuffer& framebuffer, const glm::vec4& clip) {
	float xs{ static_cast<float>(framebuffer.width() * (clip.x + 1) / 2.0) };
	float ys{ static_cast<float>(framebuffer.height() - framebuffer.height() * (clip.y + 1) / 2.0) };
	return glm::vec2{ xs, ys };
}

float edge(glm::vec2 v0, glm::vec2 v1, glm::vec2 p) {
	// 2D cross product (v1 - v0) x (p - v0)
	return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x);
};

bool insideBarycentric(glm::vec2 p,
	glm::vec2 a, glm::vec2 b, glm::vec2 c,
	float& lambda1, float& lambda2, float& lambda3)
{
	// Sample at pixel center
	glm::vec2 pf{ static_cast<float>(p.x) + 0.5f, static_cast<float>(p.y) + 0.5f };
	glm::vec2 af{ static_cast<float>(a.x), static_cast<float>(a.y) };
	glm::vec2 bf{ static_cast<float>(b.x), static_cast<float>(b.y) };
	glm::vec2 cf{ static_cast<float>(c.x), static_cast<float>(c.y) };

	float area{ edge(af, bf, cf) };
	if (area == 0.0f) {
		lambda1 = lambda2 = lambda3 = 0.0f;
		return false;
	}

	// Unnormalized barycentrics (edge function areas)
	float w1{ edge(bf, cf, pf) }; // corresponds to vertex a
	float w2{ edge(cf, af, pf) }; // corresponds to vertex b
	float w3{ edge(af, bf, pf) }; // corresponds to vertex c

	// Inside test that works for either winding:
	bool hasNeg{ (w1 < 0.0f) || (w2 < 0.0f) || (w3 < 0.0f) };
	bool hasPos{ (w1 > 0.0f) || (w2 > 0.0f) || (w3 > 0.0f) };
	if (hasNeg && hasPos) {
		return false;
	}

	// Normalize to get lambdas
	lambda1 = w1 / area;
	lambda2 = w2 / area;
	lambda3 = w3 / area;

	// Optional: small numerical cleanup
	// (keeps sum near 1 even with float error)
	// float sum = lambda1 + lambda2 + lambda3;
	// lambda1 /= sum; lambda2 /= sum; lambda3 /= sum;

	return true;
}

struct EdgeEq {
	float A, B, C;
	inline float eval(float x, float y) const { return A * x + B * y + C; }
};

inline EdgeEq makeEdge(glm::vec2 v0, glm::vec2 v1) {
	EdgeEq e {
		v1.y - v0.y,
		-(v1.x - v0.x),
		v1.x * v0.y - v1.y * v0.x 
	};
	return e;
}

void fillTriangle(Framebuffer& framebuffer, std::vector<float>& depth,
	glm::vec2 screenA, glm::vec2 screenB, glm::vec2 screenC,
	const Vertex3D& vertexA, const Vertex3D& vertexB, const Vertex3D& vertexC,
	glm::vec4 clipA, glm::vec4 clipB, glm::vec4 clipC,
	float invWA, float invWB, float invWC, const StbImage& texture) {

	// compute the bounding box of the triangle
	int minX{ static_cast<int>(std::floor(std::min({ screenA.x, screenB.x, screenC.x }))) };
	int maxX{ static_cast<int>(std::ceil(std::max({ screenA.x, screenB.x, screenC.x }))) };
	int minY{ static_cast<int>(std::floor(std::min({ screenA.y, screenB.y, screenC.y }))) };
	int maxY{ static_cast<int>(std::ceil(std::max({ screenA.y, screenB.y, screenC.y }))) };
	const int W{ framebuffer.width() };
	const int H{ framebuffer.height() };
	minX = std::max(minX, 0);
	minY = std::max(minY, 0);
	maxX = std::min(maxX, W - 1);
	maxY = std::min(maxY, H - 1);
	if (minX > maxX || minY > maxY) return;

	int32_t texW{ static_cast<int32_t>(texture.getWidth()) };
	int32_t texH{ static_cast<int32_t>(texture.getHeight()) };

	// Pixel-center sampling: add 0.5 here ONCE (not in a function called per pixel)
	float startX{ minX + 0.5f };
	float startY{ minY + 0.5f };

	// Edge equations (opposite each vertex)
	EdgeEq eA{ makeEdge(screenB, screenC) }; // wA
	EdgeEq eB{ makeEdge(screenC, screenA) }; // wB
	EdgeEq eC{ makeEdge(screenA, screenB) }; // wC

	// Triangle area (same as eC evaluated at C, etc.)
	float area{ eC.eval(screenC.x, screenC.y) };
	if (area == 0.0f) return;

	// Optional: enforce consistent winding so inside test is one-sided (faster)
	// If area < 0, flip signs so "inside" is (w >= 0)
	if (area < 0.0f) {
		area = -area;
		eA.A = -eA.A; eA.B = -eA.B; eA.C = -eA.C;
		eB.A = -eB.A; eB.B = -eB.B; eB.C = -eB.C;
		eC.A = -eC.A; eC.B = -eC.B; eC.C = -eC.C;
	}

	// Evaluate edge funcs at start of bbox
	float wA_row{ eA.eval(startX, startY) };
	float wB_row{ eB.eval(startX, startY) };
	float wC_row{ eC.eval(startX, startY) };

	// Per-pixel step in X is just adding A; per-row step in Y is adding B
	float wA_stepX{ eA.A }, wA_stepY{ eA.B };
	float wB_stepX{ eB.A }, wB_stepY{ eB.B };
	float wC_stepX{ eC.A }, wC_stepY{ eC.B };

	// loop through the bounding box, and test each pixel to see if it's inside the triangle
	for (int y{ minY }; y <= maxY; ++y) {
		float wA{ wA_row };
		float wB{ wB_row };
		float wC{ wC_row };
		const size_t rowStart{ static_cast<size_t>(y) * static_cast<size_t>(W) + static_cast<size_t>(minX) };

		for (int x{ minX }, offset{ 0 }; x <= maxX; ++x, ++offset) {
			const size_t idx{ rowStart + static_cast<size_t>(offset) };
			// Inside test (now one-sided because we normalized winding above)
			if (wA >= 0.0f && wB >= 0.0f && wC >= 0.0f) {
				// Barycentrics
				float lambda1{ wA / area };
				float lambda2{ wB / area };
				float lambda3{ wC / area };

				// ... do invW/zOverW depth test then UV sample ...
				float invW{ lambda1 * invWA + lambda2 * invWB + lambda3 * invWC };
				if (!(invW > 0.0f)) {
					continue; // skip degenerate or behind-camera fragments
				}

				
				// depth as NDC z (requires clip z values that match invW inputs)
				float zNdc {(lambda1 * (clipA.z * invWA) +
					lambda2 * (clipB.z * invWB) +
					lambda3 * (clipC.z * invWC)) / invW};
				float depthValue = zNdc * 0.5f + 0.5f; // OpenGL NDC -> [0,1], near -> 0

				if (depthValue< depth[idx]) {
					// perspective-correct UV
					float u{ (lambda1 * (vertexA.u * invWA) +
						lambda2 * (vertexB.u * invWB) +
						lambda3 * (vertexC.u * invWC)) / invW };

					float v{ (lambda1 * (vertexA.v * invWA) +
						lambda2 * (vertexB.v * invWB) +
						lambda3 * (vertexC.v * invWC)) / invW };

					depth[idx] = depthValue;

					u = std::clamp(u, 0.0f, 1.0f);
					v = std::clamp(v, 0.0f, 1.0f); // or v = 1.0f - v

					int tu{ std::clamp((int)std::floor(u * (texW - 1)), 0, texW - 1) };
					int tv{ std::clamp((int)std::floor(v * (texH - 1)), 0, texH - 1) };

					int channels{ 4 };// texture.getBpp(); // verify: must be 3 or 4 BYTES per pixel
					int tIdx{ (tv * texW + tu) * channels };

					framebuffer.data()[idx] = Pixel{
						texture.getData()[tIdx + 0],
						texture.getData()[tIdx + 1],
						texture.getData()[tIdx + 2],
						255
					};
				}
			}
			wA += wA_stepX;
			wB += wB_stepX;
			wC += wC_stepX;
		}
		wA_row += wA_stepY;
		wB_row += wB_stepY;
		wC_row += wC_stepY;
	}
}

void drawMesh(Framebuffer& framebuffer, std::vector<float>& depth,
	const glm::mat4& modelMatrix,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const std::vector<Vertex3D>& vertices,
	const std::vector<uint32_t>& faces,
	const StbImage &texture) {

	// TODO: first, construct a new 4x4 "MVP" matrix, by multiplying the
	// model, view, and projection matrices as shown in lecture. The order matters!!!
	glm::mat4 mvp{ projectionMatrix * viewMatrix * modelMatrix };

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
		glm::vec4 clipA{ mvp* glm::vec4{localA.x, localA.y, localA.z, 1.0f} };
		glm::vec4 clipB{ mvp * glm::vec4{localB.x, localB.y, localB.z, 1.0f} };
		glm::vec4 clipC{ mvp * glm::vec4{localC.x, localC.y, localC.z, 1.0f } };

		float invWA{ 1 / clipA.w };
		float invWB{ 1 / clipB.w };
		float invWC{ 1 / clipC.w };
		
		clipA /= clipA.w;
		clipB /= clipB.w;
		clipC /= clipC.w;
		auto screenA{ clipToScreen(framebuffer, clipA) };
		auto screenB{ clipToScreen(framebuffer, clipB) };
		auto screenC{ clipToScreen(framebuffer, clipC) };

		fillTriangle(framebuffer, depth, screenA, screenB, screenC, localA, localB, localC, clipA, clipB, clipC, invWA, invWB, invWC, texture);
	}
}


int main() {
	sf::RenderWindow window{ sf::VideoMode::getFullscreenModes().at(0), "SFML Demo" };
	const auto windowSize{ window.getSize() };
	Framebuffer framebuffer{ static_cast<int>(windowSize.x), static_cast<int>(windowSize.y) };

	//std::vector<Vertex3D> bunnyVertices{ {-0.5, -0.5, 0, 1, 1}, {-0.5, 0.5, 0, .5, .5 }, {0.5, 0.5, 0., 1, 0.} };
	//std::vector<uint32_t> bunnyFaces{0, 1, 2};
	std::vector<Vertex3D> bunnyVertices{};
	std::vector<uint32_t> bunnyFaces{};
	assimpLoad("models/bunny_textured.obj", bunnyVertices, bunnyFaces);
	StbImage tex;
	//tex.loadFromFile("models/wall.jpg");
	tex.loadFromFile("models/bunny_textured.jpg");

	glm::vec3 bunnyPosition{ 0, -1, -3.5 };
	glm::vec3 bunnyOrientation{ 0, 0, 0 };
	glm::vec3 bunnyScale{ 9, 9, 9 };

	float fovy{ 60.0f };
	float ratio{ static_cast<float>(windowSize.x) / (windowSize.y) };
	float near{ 0.1f };
	float far{ 100.0f };
	float t{ static_cast<float>(near * tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2)) };
	float b{ -t };
	float r{ t * ratio };
	float l{ -r };

	std::vector<float> depth(windowSize.x * windowSize.y);
	sf::Texture framebufferTexture(windowSize);
	framebufferTexture.setSmooth(false);
	sf::Sprite framebufferSprite(framebufferTexture);

	while (window.isOpen()) {
		// Check for events.
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}
		framebuffer.clear(Pixel{});
		std::fill(depth.begin(), depth.end(), 1.0f);

		//initialize a vector of size width*height to hold depth values for each pixel, and initialize them to some very large value (e.g. 1000.0f)


		// Rotate the bunny by incrementing the orientation. This is a "yaw" around the y axis.
		bunnyOrientation.y += 0.001f;

		glm::mat4 bunnyModelMatrix{ buildModelMatrix(bunnyPosition, bunnyOrientation, bunnyScale) };
		glm::mat4 viewMatrix{ 1 }; // identity matrix == camera at origin, looking down -z axis.
		glm::mat4 projectionMatrix{ glm::perspective(glm::radians(60.0f), windowSize.x / (float)windowSize.y, 0.1f, 100.0f)};

		// Render the scene.
		drawMesh(framebuffer, depth, bunnyModelMatrix, viewMatrix, projectionMatrix, bunnyVertices, bunnyFaces, tex);
		framebufferTexture.update(reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
		window.clear(sf::Color::Black);
		window.draw(framebufferSprite);
		window.display();
	}

	return 0;
}
