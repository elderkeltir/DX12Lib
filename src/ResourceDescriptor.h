#pragma once

#include <directx/d3dx12.h>

class HeapBuffer;
struct DSVdesc;
struct SRVdesc;
struct UAVdesc;
struct CBVdesc;

class ResourceDescriptor{
public:
    enum class ResourceDescriptorType { rdt_rtv, rdt_dsv, rdt_srv, rdt_uav, rdt_cbv };
public:
    bool Create_RTV(std::weak_ptr<HeapBuffer> buff);
    bool Create_DSV(std::weak_ptr<HeapBuffer> buff, const DSVdesc &desc);
    bool Create_SRV(std::weak_ptr<HeapBuffer> buff, const SRVdesc &desc);
    bool Create_UAV(std::weak_ptr<HeapBuffer> buff, const UAVdesc &desc);
    bool Create_CBV(std::weak_ptr<HeapBuffer> buff, const CBVdesc &desc);

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUhandle() const { return m_cpu_handle; }
    ResourceDescriptorType GetType() const { return m_type; }
private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;

    ResourceDescriptorType m_type;
};