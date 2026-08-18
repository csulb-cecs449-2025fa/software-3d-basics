#pragma once
#include <vector>

struct Vertex3D {
	float x;
	float y;
	float z;
	float u;
	float v;
};

class Mesh {
private:
	std::vector<Vertex3D> m_vertices;
	std::vector<uint32_t> m_faces;
};