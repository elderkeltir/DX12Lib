#pragma once 

#include "IDynamicGpuHeap.h"
#include "ICommandQueue.h"
#include <vector>
#include <volk.h>

class IHeapBuffer;

class DynamicGpuHeap : public IDynamicGpuHeap {
public:
    void Initialize(uint32_t queue_id) override;
    void CacheRootSignature(const IRootSignature* root_sig, uint32_t tech_id = 0) override;
    void StageDesctriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IResourceDescriptor>& desc_handle) override;
    void ReserveDescriptor(CPUdescriptor& cpu_descriptor, GPUdescriptor& gpu_descriptor) override;
    void CommitRootSignature(ICommandList* command_list, bool gfx = true) override;
    void Reset() override;

    void StageDescriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IHeapBuffer>& buff_handle);
private:
    ICommandQueue::QueueType m_queue_id; // TODO: gfx=0, compute=1
    std::vector<VkWriteDescriptorSet> m_cached_writes;
    std::vector<std::vector<VkDescriptorBufferInfo>> m_cached_writes_buffers;
    std::vector<std::vector<VkDescriptorImageInfo>> m_cached_writes_images;
    VkDescriptorSet m_descriptor_set;
    RootSignature* m_root_sig;
    uint32_t m_tech_id;
};
