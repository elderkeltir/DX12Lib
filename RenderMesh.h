#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <array>
#include <string>

class Transformations;

class RenderMesh {
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
    RenderMesh();
    ~RenderMesh();

    void Load(const std::wstring &name);
    void SetName(const std::wstring &name);
    const std::wstring& GetName() const;

    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &angles);
    void Scale(const DirectX::XMFLOAT3 &scale);

    void SetVertices(std::vector<DirectX::XMFLOAT3> vertices);
    void SetIndices(std::vector<uint32_t> indices);
    void SetNormals(std::vector<DirectX::XMFLOAT3> normals);
    void SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents);

    void SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords);
    void SetTexturePath(const char* path, TextureType type);
private:
    std::vector<DirectX::XMFLOAT3> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;
    std::vector<DirectX::XMFLOAT2> m_textCoords;

    std::vector<Vertex> m_vertexDataBuffer;

    std::string m_vertexShaderName;
    std::string m_pixelShaderName;
    std::array<std::string, 3> m_textures;

    std::unique_ptr<Transformations> m_transformations;

    std::wstring m_name;
};