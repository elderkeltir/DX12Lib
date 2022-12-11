#pragma once

#include <DirectXMath.h>
#include <vector>


inline void CreateSphere(std::vector<DirectX::XMFLOAT3> &vertices, std::vector<uint16_t> &indices)
{
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.

	float radius = 0.5f;
	int stackCount = 20;
	int sliceCount = 20;

	DirectX::XMFLOAT3 topVertex(0.0f, +radius, 0.0f);
	DirectX::XMFLOAT3 bottomVertex(0.0f, -radius, 0.0f);

	vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (int i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (int j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DirectX::XMFLOAT3 v;

			// spherical to cartesian
			v.x = radius * sinf(phi)*cosf(theta);
			v.y = radius * cosf(phi);
			v.z = radius * sinf(phi)*sinf(theta);

			vertices.push_back(v);
		}
	}

	vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (int i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	int southPoleIndex = (int)indices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (int i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}

inline void CreateQuad(std::vector<DirectX::XMFLOAT3> &vertices, std::vector<DirectX::XMFLOAT2> &tex_coords, std::vector<uint16_t> &indices){
    vertices.push_back(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f));

    tex_coords.push_back(DirectX::XMFLOAT2(0.0f, 1.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(1.0f, 0.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(1.0f, 1.0f));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);
}

inline void CreateTriangle(std::vector<uint16_t>& indices) {
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
}