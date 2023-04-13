#pragma once

#include "IHeapBuffer.h"
#include "VkMemoryHelper.h"
#include "vk_helper.h"

class HeapBuffer : public IHeapBuffer {
public:
    enum BufferResourceType { rt_undef, rt_buffer, rt_texture };

    void Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name = std::nullopt) override;

    void Load(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) override;
    void Load(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) override;

    void* Map() override;
    void Unmap() override;
    void* GetCpuData() override { return m_cpu_data; }

    BufferResourceType GetVkType() const {
        return m_type;
    }

    const BufferMemAllocation& GetBufferInfo() const {
        assert(m_type == BufferResourceType::rt_buffer);

        return m_buffer_allocation;
    }

    const ImageMemAllocation& GetImageInfo() const {
        assert(m_type == BufferResourceType::rt_texture);

        return m_image_allocation;
    }

    void SetImage(VkImage image, VkFormat format, VkImageAspectFlags aspect);

private:
    BufferResourceType m_type { BufferResourceType::rt_undef };
    ImageMemAllocation m_image_allocation;
    BufferMemAllocation m_buffer_allocation;

    void* m_cpu_data{nullptr};
};