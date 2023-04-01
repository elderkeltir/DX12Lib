#pragma once

#include <memory>
#include "defines.h"

class IHeapBuffer;
struct DSVdesc;
struct SRVdesc;
struct UAVdesc;
struct CBVdesc;

class IResourceDescriptor {
public:
    enum class ResourceDescriptorType { rdt_rtv, rdt_dsv, rdt_srv, rdt_uav, rdt_cbv };

    virtual bool Create_RTV(std::weak_ptr<IHeapBuffer> buff) = 0;
    virtual bool Create_DSV(std::weak_ptr<IHeapBuffer> buff, const DSVdesc& desc) = 0;
    virtual bool Create_SRV(std::weak_ptr<IHeapBuffer> buff, const SRVdesc& desc) = 0;
    virtual bool Create_UAV(std::weak_ptr<IHeapBuffer> buff, const UAVdesc& desc) = 0;
    virtual bool Create_CBV(std::weak_ptr<IHeapBuffer> buff, const CBVdesc& desc) = 0;

    virtual CPUdescriptor GetCPUhandle() const = 0;
    virtual ResourceDescriptorType GetType() const = 0;
};