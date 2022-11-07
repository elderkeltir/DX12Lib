#pragma once

#include "RenderObject.h"
#include <optional>

class RenderQuad : public RenderObject {
public:
    ~RenderQuad();
    void Initialize();

    virtual void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &command_list) override;
    void CreateQuadTexture(uint32_t width, uint32_t height, const std::vector<DXGI_FORMAT> &formats, uint32_t texture_num, std::optional<std::wstring> dbg_name = std::nullopt);

    void SetSrv(ComPtr<ID3D12GraphicsCommandList6> &command_list, uint32_t set_idx, uint32_t root_idx, uint32_t idx_in_set = 0u);
    std::weak_ptr<GpuResource> GetRt(uint32_t set_idx, uint32_t idx_in_set = 0u);
    std::vector< std::shared_ptr<GpuResource>>& GetRts(uint32_t set_idx) { return m_textures.at(set_idx); }
    void Render(ComPtr<ID3D12GraphicsCommandList6> &command_list);
private:
    
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 textCoord;
    };

    void FormVertex();

    std::vector<Vertex> m_vertexDataBuffer;
    std::vector<std::vector<std::shared_ptr<GpuResource>>> m_textures;
};