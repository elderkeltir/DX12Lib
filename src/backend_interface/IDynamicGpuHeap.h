#pragma once

#include <memory>
#include "defines.h"

class RootSignature;
class ICommandList;
class IResourceDescriptor;
class RootSignature;
class IRootSignature;

class IDynamicGpuHeap {
public:
    virtual void Initialize(uint32_t frame_id) = 0;
    virtual void CacheRootSignature(const IRootSignature* root_sig) = 0;
    virtual void StageDesctriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IResourceDescriptor>& desc_handle) = 0;
    virtual void ReserveDescriptor(CPUdescriptor& cpu_descriptor, GPUdescriptor& gpu_descriptor) = 0;
    virtual void CommitRootSignature(ICommandList* command_list, bool gfx = true) = 0;
    virtual void Reset() = 0;
    virtual ~IDynamicGpuHeap()  = default;
};