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

    void AddChild(RenderModel* child) { m_children.push_back(child); }
    RenderModel* GetChild(uint32_t idx) { return m_children[idx]; }

    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &angles);
    void Scale(const DirectX::XMFLOAT3 &scale);

    void SetTexture(TextureData * texture_data, TextureType type) override;
    void SetTechniqueId(uint32_t id) { m_tech_id = id; for(auto &child : m_children) child->SetTechniqueId(id); }
    void SetColor(const DirectX::XMFLOAT3 &color) { m_color = color; for(auto &child : m_children) child->SetColor(color); }
    void SetMaterial(uint32_t id) { m_material_id = id; for(auto &child : m_children) child->SetMaterial(id); }

private:
    inline void FormVertexes();
    inline void LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & commandList);
    inline void LoadConstantData(ComPtr<ID3D12GraphicsCommandList6> &command_list);

    std::unique_ptr<GpuResource> m_constant_buffer;
    std::unique_ptr<GpuResource> m_diffuse_tex;
    std::unique_ptr<GpuResource> m_normals_tex;
    std::unique_ptr<GpuResource> m_specular_tex;
    std::array<TextureData*, TextureCount> m_textures_data;
    std::unique_ptr<Transformations> m_transformations;
    std::vector<RenderModel*> m_children;
    uint32_t m_tech_id{uint32_t(-1)};
    DirectX::XMFLOAT3 m_color{0.5f,0.3f,0.7f};
    uint32_t m_material_id{0};
};