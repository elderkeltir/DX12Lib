#pragma once

#include <array>
#include "simple_object_pool.h"
#include "RenderObject.h"

class Transformations;

class RenderModel : public RenderObject {
public:
    RenderModel();
    ~RenderModel();

    void Load(const std::wstring &name);
    void Render(ComPtr<ID3D12GraphicsCommandList6> &commandList, const DirectX::XMFLOAT4X4 &parent_xform);
    virtual void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList) override;

    void AddChild(RenderModel * child) { m_children.push_back(child); }
    const std::vector<RenderModel*>& GetChildren() const { return m_children; }

    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &angles);
    void Scale(const DirectX::XMFLOAT3 &scale);

    void SetNormals(std::vector<DirectX::XMFLOAT3> normals);
    void SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents);
    void SetTexture(TextureData * texture_data, TextureType type);
private:
    struct Vertex
	{
		DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangents;
		DirectX::XMFLOAT3 Bitangents;
        DirectX::XMFLOAT2 TextCoord;
	};
    
    inline void FormVertexes();
    inline void LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & commandList);

    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;

    std::vector<Vertex> m_vertexDataBuffer;
    std::unique_ptr<GpuResource> m_diffuse_tex;
    std::unique_ptr<GpuResource> m_normals_tex;
    std::unique_ptr<GpuResource> m_specular_tex;
    std::array<TextureData*, TextureCount> m_textures_data;
    std::unique_ptr<Transformations> m_transformations;
    std::vector<RenderModel*> m_children;
};