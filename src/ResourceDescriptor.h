#pragma once

#include <directx/d3dx12.h>

class HeapBuffer;

class ResourceDescriptor{
public:
    enum class ResourceDescriptorType { rdt_rtv, rdt_dsv, rdt_srv, rdt_uav, rdt_cbv };
public:
    bool Create_RTV(std::weak_ptr<HeapBuffer> buff);
    bool Create_DSV(std::weak_ptr<HeapBuffer> buff, const D3D12_DEPTH_STENCIL_VIEW_DESC &desc);
    bool Create_SRV(std::weak_ptr<HeapBuffer> buff, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
    bool Create_UAV(std::weak_ptr<HeapBuffer> buff, const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc);
    bool Create_CBV(std::weak_ptr<HeapBuffer> buff, const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc);

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUhandle() const { return m_cpu_handle; }
    ResourceDescriptorType GetType() const { return m_type; }
private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;

    ResourceDescriptorType m_type;
};