#include "DynamicGpuHeap.h"
#include "RootSignature.h"
#include "vk_helper.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "DescriptorHeapCollection.h"
#include "ResourceDescriptor.h"
#include "CommandList.h"
#include "HeapBuffer.h"
#include "Techniques.h"

extern VkBackend* gBackend;

void DynamicGpuHeap::Initialize(uint32_t queue_id) {
    m_queue_id = (ICommandQueue::QueueType)queue_id;
}

void DynamicGpuHeap::CacheRootSignature(const IRootSignature* root_sig, uint32_t tech_id) {
    m_tech_id = tech_id;
    RootSignature* root_signature = (RootSignature*)root_sig;
    m_root_sig = root_signature;
    const std::vector<VkDescriptorSetLayoutBinding>& root_sig_layout = root_signature->GetBindings();
    VkDescriptorSetLayout desc_set_layout = root_signature->GetLayout();
    
    // Create desc set
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
    VkDescriptorPool pool = ((DescriptorHeapCollection*)gBackend->GetDescriptorHeapCollection())->GetPool(gBackend->GetCurrentBackBufferIndex(), (uint32_t)m_queue_id);

	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1u;
	allocInfo.pSetLayouts = &desc_set_layout;
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &m_descriptor_set));

    // cache layout
    m_cached_writes.resize(root_sig_layout.size());
    uint32_t sampler_id = 0;
    for (uint32_t i = 0; i < root_sig_layout.size(); i++) {
        m_cached_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_cached_writes[i].dstSet = m_descriptor_set;
        m_cached_writes[i].dstBinding = root_sig_layout[i].binding;
        m_cached_writes[i].dstArrayElement = 0;
        m_cached_writes[i].descriptorCount = root_sig_layout[i].descriptorCount;
        m_cached_writes[i].descriptorType = root_sig_layout[i].descriptorType;
        m_cached_writes[i].descriptorType = root_sig_layout[i].descriptorType;
        if (m_cached_writes[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) {
            VkSampler sampler = gBackend->GetSamplerById(sampler_id); // TODO: implement this
            VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = sampler;

            m_cached_writes[1].pImageInfo = &imageInfo;

            sampler_id++;
        }
    }
}

void DynamicGpuHeap::StageDesctriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IResourceDescriptor>& desc_handle) {
    ResourceDescriptor* res_hndl = (ResourceDescriptor*) desc_handle.get();

    // Find descriptor using bind_point
    uint32_t idx = 0;
    for (; idx < m_cached_writes.size(); idx++) {
        IResourceDescriptor::ResourceDescriptorType type = desc_handle->GetType();
        uint32_t bind_point = m_root_sig->GetConverter().Convert(type, offset);

        if (m_cached_writes[idx].dstBinding == bind_point) {
            break;
        }
    }

    assert(idx < m_cached_writes.size());

    if (m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        BufferMemAllocation* buff_data = (BufferMemAllocation*)res_hndl->GetCPUhandle().ptr;
        VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buff_data->buffer;
		bufferInfo.offset = buff_data->offset;
		bufferInfo.range = buff_data->size;

        m_cached_writes[idx].pBufferInfo = &bufferInfo;
    }
    else if (m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
        VkImageView image_view = (VkImageView)res_hndl->GetCPUhandle().ptr;
        VkDescriptorImageInfo imageInfo{};
        // TODO: check for UAV
        imageInfo.imageLayout = (m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL);
        imageInfo.imageView = image_view;
        m_cached_writes[idx].pImageInfo = &imageInfo;
    }
    else {
        assert(false);
    }
}

void DynamicGpuHeap::ReserveDescriptor(CPUdescriptor& cpu_descriptor, GPUdescriptor& gpu_descriptor) {
    assert(false);
}

void DynamicGpuHeap::CommitRootSignature(ICommandList* command_list, bool gfx) {
    // update descriptors
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
    vkUpdateDescriptorSets(device, m_cached_writes.size(), m_cached_writes.data(), 0, nullptr);

    CommandList* cmd_list = (CommandList*)command_list;
    VkPipelineLayout pipline_layout = ((Techniques::TechniqueVk*)gBackend->GetTechniqueById(m_tech_id))->pipeline_layout;
    vkCmdBindDescriptorSets(cmd_list->GetNativeObject(), gfx ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, pipline_layout, 0, 1, &m_descriptor_set, 0, nullptr);
}

void DynamicGpuHeap::Reset() {
    m_cached_writes.clear();
    m_descriptor_set = nullptr;
    m_root_sig = nullptr;
    m_tech_id = uint32_t(-1);
}

void DynamicGpuHeap::StageDescriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IHeapBuffer>& buff_handle) {
    // Find descriptor using bind_point
    uint32_t idx = 0;
    for (; idx < m_cached_writes.size(); idx++) {
        uint32_t bind_point = m_root_sig->GetConverter().Convert(IResourceDescriptor::ResourceDescriptorType::rdt_cbv, offset);
        if (m_cached_writes[idx].dstBinding == bind_point) {
            break;
        }
    }

    if (m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || m_cached_writes[idx].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        const BufferMemAllocation& buff_data = ((HeapBuffer*)buff_handle.get())->GetBufferInfo();
        VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buff_data.buffer;
		bufferInfo.offset = buff_data.offset;
		bufferInfo.range = buff_data.size;

        m_cached_writes[idx].pBufferInfo = &bufferInfo;
    }
}
