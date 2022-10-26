#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <array>
#include <string>

#include "Transformations.h"

class RenderModel {
public:
    struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TextCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangents;
		DirectX::XMFLOAT3 Bitangents;

		Vertex() {}
		Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 textCoord) : Position(pos), TextCoord(textCoord) {}
	};
public:
    enum TextureType { DiffuseTexture = 0, NormalTexture, SpecularTexture };
public:
    RenderModel();
    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &angles);
    void Scale(const DirectX::XMFLOAT3 &scale);

    void SetVertices(std::unique_ptr<DirectX::XMFLOAT3[]> vertices, uint32_t size);
    void SetIndices(std::unique_ptr<uint32_t[]> indices, uint32_t size);
    void SetNormals(std::unique_ptr<DirectX::XMFLOAT3[]> normals);
    void SetTangents(std::unique_ptr<DirectX::XMFLOAT3[]> tangents, std::unique_ptr<DirectX::XMFLOAT3[]> bitangents);

    void SetTextureCoords(std::unique_ptr<DirectX::XMFLOAT2[]> textCoords);
    void SetTexturePath(const char* path, TextureType type);
private:
    std::unique_ptr<DirectX::XMFLOAT3[]> m_vertices;
    std::unique_ptr<uint32_t[]> m_indices;
    std::unique_ptr<DirectX::XMFLOAT3[]> m_normals;
    std::unique_ptr<DirectX::XMFLOAT3[]> m_tangents;
    std::unique_ptr<DirectX::XMFLOAT3[]> m_bitangents;
    std::unique_ptr<DirectX::XMFLOAT2[]> m_textCoords;

    std::vector<Vertex> m_vertexDataBuffer;

    std::string m_vertexShaderName;
    std::string m_pixelShaderName;
    std::array<std::string, 3> m_textures;

    std::unique_ptr<Transformations> m_transformations;

    int m_vertexCnt;
    int m_indexCnt;
};