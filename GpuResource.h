#pragma once

#include <memory>

#include "HeapBuffer.h"
class ResourceDescriptor;

class GpuResource {
public:
    ~GpuResource();
    void CreateBuffer(HeapBuffer::BufferType type, uint32_t bufferSize, HeapBuffer::UseFlag flag, D3D12_RESOURCE_STATES initial_state, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateTexture(HeapBuffer::BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE *clear_val, std::optional<std::wstring> dbg_name = std::nullopt);
    void SetBuffer(ComPtr<ID3D12Resource> res);
    void LoadBuffer(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t numElements, uint32_t elementSize, const void* bufferData);
    void LoadBuffer(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);
    
    void CreateRTV();
    void Create_DSV(const D3D12_DEPTH_STENCIL_VIEW_DESC &desc);
    void Create_SRV(const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool gpu_visible = true);
    void Create_UAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc, bool gpu_visible = true);
    void Create_CBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc, bool gpu_visible = true);
    void Create_Vertex_View(uint32_t sizSizeInBytese, uint32_t StrideInBytes);
    void Create_Index_View(DXGI_FORMAT format, uint32_t SizeInBytes);

    std::weak_ptr<HeapBuffer> GetBuffer() { return m_buffer; }
    std::weak_ptr<ResourceDescriptor> GetRTV() { return m_rtv; }
    std::weak_ptr<ResourceDescriptor> GetDSV() { return m_dsv; }
    std::weak_ptr<ResourceDescriptor> GetSRV() { return m_srv; }
    std::weak_ptr<ResourceDescriptor> GetCBV() { return m_cbv; }
    std::weak_ptr<D3D12_VERTEX_BUFFER_VIEW> Get_Vertex_View() { return m_vertex_view; }
    std::weak_ptr<D3D12_INDEX_BUFFER_VIEW> Get_Index_View() { return m_index_view; }
private:
    void ResetViews();
    std::shared_ptr<HeapBuffer> m_buffer;
    std::shared_ptr<ResourceDescriptor> m_rtv;
    std::shared_ptr<ResourceDescriptor> m_dsv;
    std::shared_ptr<ResourceDescriptor> m_srv;
    std::shared_ptr<ResourceDescriptor> m_uav;
    std::shared_ptr<ResourceDescriptor> m_cbv;
    std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> m_vertex_view;
    std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> m_index_view;
};