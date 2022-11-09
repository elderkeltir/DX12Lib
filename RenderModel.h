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
    void SetTechniqueId(uint32_t id) { m_tech_id = id; for(auto &child : m_children) child->SetTechniqueId(id); }
private:
    inline void FormVertexes();
    inline void LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & commandList);

    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;

    std::unique_ptr<GpuResource> m_diffuse_tex;
    std::unique_ptr<GpuResource> m_normals_tex;
    std::unique_ptr<GpuResource> m_specular_tex;
    std::array<TextureData*, TextureCount> m_textures_data;
    std::unique_ptr<Transformations> m_transformations;
    std::vector<RenderModel*> m_children;
    uint32_t m_tech_id{uint32_t(-1)};
};