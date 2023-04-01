#pragma once

#include <string>
#include <DirectXMath.h>
#include <directx/d3d12.h>
#include <wrl.h>
#include "simple_object_pool.h"

using Microsoft::WRL::ComPtr;

class RenderModel;
class ICommandList;

class LevelEntity {
public:
    LevelEntity() = default;
    LevelEntity(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &rot, const DirectX::XMFLOAT3 &scale);
    virtual void Load(const std::wstring &name);
    virtual void Update(float dt);
    virtual void Render(ICommandList* command_list);
    const DirectX::XMFLOAT4X4& GetXform() const { return m_xform; }
    virtual uint32_t GetTechniqueId() const { return m_tech_id; }
    void SetId(uint32_t id) { m_id = id; }
    uint32_t GetId() const { return m_id; }
    void LoadDataToGpu(ICommandList* command_list);
    void SetPos(const DirectX::XMFLOAT3& pos) { m_pos = pos; }
    void SetRot(const DirectX::XMFLOAT3& rot) { m_rot = rot; }
    void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; }
    virtual ~LevelEntity() = default;
protected:
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