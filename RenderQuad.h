#pragma once

#include "RenderObject.h"

class RenderQuad : public RenderObject {
public:
    ~RenderQuad();
    void Initialize(uint32_t texture_num);

    virtual void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &command_list) override;
    void CreateQuadTexture(uint32_t width, uint32_t height);

    void SetSrv(ComPtr<ID3D12GraphicsCommandList6> &command_list, uint32_t idx);
    std::weak_ptr<GpuResource> GetRt(uint32_t idx);
    void Render(ComPtr<ID3D12GraphicsCommandList6> &command_list);
private:
    
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 textCoord;
    };

    void FormVertex();

    std::vector<Vertex> m_vertexDataBuffer;
    std::vector<std::shared_ptr<GpuResource>> m_textures;
    uint32_t m_texture_num;
};