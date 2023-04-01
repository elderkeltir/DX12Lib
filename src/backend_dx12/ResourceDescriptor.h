#pragma once

#include "IResourceDescriptor.h"

class ResourceDescriptor : public IResourceDescriptor{
public:
    bool Create_RTV(std::weak_ptr<IHeapBuffer> buff) override;
    bool Create_DSV(std::weak_ptr<IHeapBuffer> buff, const DSVdesc &desc) override;
    bool Create_SRV(std::weak_ptr<IHeapBuffer> buff, const SRVdesc &desc) override;
    bool Create_UAV(std::weak_ptr<IHeapBuffer> buff, const UAVdesc &desc) override;
    bool Create_CBV(std::weak_ptr<IHeapBuffer> buff, const CBVdesc &desc) override;

    CPUdescriptor GetCPUhandle() const override { return m_cpu_handle; }
    ResourceDescriptorType GetType() const override { return m_type; }
private:
    CPUdescriptor m_cpu_handle;

    ResourceDescriptorType m_type;
};