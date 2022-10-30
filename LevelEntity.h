#pragma once

#include <string>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class RenderModel;

class LevelEntity {
public:
    LevelEntity() = default;
    LevelEntity(const std::wstring &model_name, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT3 scale);
    void Load(const std::wstring &name);
    void Update(float dt);
    void Render(ComPtr<ID3D12GraphicsCommandList6> &commandList);
    const DirectX::XMFLOAT4X4& GetXform() const { return m_xform; }
    uint32_t GetTechniqueId() const { return m_tech_id; }
    void SetId(uint32_t id) { m_id = id; }
    uint32_t GetId() const { return m_id; }
   
private:
    std::wstring m_model_name;
    RenderModel* m_model;
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_rot;
    DirectX::XMFLOAT3 m_scale;
    uint32_t m_id;
    uint32_t m_tech_id{(uint32_t)(-1)};
    //
    DirectX::XMFLOAT4X4 m_xform;
};