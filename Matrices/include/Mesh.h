#pragma once
#include <vector>

struct Vertex3D {
	float x;
	float y;
	float z;
};

class Mesh {
private:
	std::vector<Vertex3D> m_vertices;
	std::vector<uint32_t> m_faces;
};