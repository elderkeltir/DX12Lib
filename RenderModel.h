#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Transformations;
class GpuResource;

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
    ~RenderModel();

    void Load(const std::wstring &name);
    inline void LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);
    inline void LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);

    void Render(ComPtr<ID3D12GraphicsCommandList6> &commandList);

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
    enum dirty_bits{
        db_vertex       = 1 << 0,
        db_index        = 1 << 1
    };
    std::vector<DirectX::XMFLOAT3> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;
    std::vector<DirectX::XMFLOAT2> m_textCoords;

    std::unique_ptr<GpuResource> m_VertexBuffer;
    std::unique_ptr<GpuResource> m_IndexBuffer;
    //std::vector<Vertex> m_vertexDataBuffer;

    std::string m_vertexShaderName;
    std::string m_pixelShaderName;
    std::array<std::string, 3> m_textures;

    uint8_t m_dirty{ db_vertex | db_index};

    std::unique_ptr<Transformations> m_transformations;

    std::wstring m_name;
};