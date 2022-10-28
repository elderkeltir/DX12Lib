#pragma once

#include <memory>

#include "HeapBuffer.h"
class ResourceDescriptor;

class GpuResource {
public:
    void CreateBuffer(HeapBuffer::BufferType type, uint32_t bufferSize, HeapBuffer::UseFlag flag, D3D12_RESOURCE_STATES initial_state);
    void CreateTexture(HeapBuffer::BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE &clear_val);
    void SetBuffer(ComPtr<ID3D12Resource> res);
    void CreateRTV();
    void Create_DSV(const D3D12_DEPTH_STENCIL_VIEW_DESC &desc);
    void Create_SRV(const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool gpu_visible = true);
    void Create_UAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc, bool gpu_visible = true);

    std::weak_ptr<HeapBuffer> GetBuffer() { return m_buffer; }
    std::weak_ptr<ResourceDescriptor> GetRTV() { return m_rtv; }
    std::weak_ptr<ResourceDescriptor> GetDSV() { return m_dsv; }
    std::weak_ptr<ResourceDescriptor> GetSRV() { return m_srv; }

private:
    std::shared_ptr<HeapBuffer> m_buffer;
    std::shared_ptr<ResourceDescriptor> m_rtv;
    std::shared_ptr<ResourceDescriptor> m_dsv;
    std::shared_ptr<ResourceDescriptor> m_srv;
    std::shared_ptr<ResourceDescriptor> m_uav;
};