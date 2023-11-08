#include "CommandList.h"
#include "Techniques.h"
#include "vk_helper.h"
#include "VkGpuResource.h"
#include "HeapBuffer.h"
#include "CommandQueue.h"
#include "DynamicGpuHeap.h"
#include "VkBackend.h"

extern VkBackend * gBackend;

void ConvertResourceState(ResourceState from, VkImageLayout &layout, VkAccessFlags &access_mask) {
    switch(from) {
        case rs_resource_state_index_buffer:
            access_mask = VK_ACCESS_INDEX_READ_BIT;
            break;
        case rs_resource_state_render_target:
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case rs_resource_state_unordered_access:
        case rs_resource_state_depth_write:
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case rs_resource_state_depth_read:
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case rs_resource_state_non_pixel_shader_resource:
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            access_mask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case rs_resource_state_pixel_shader_resource:
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            access_mask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case rs_resource_state_stream_out:
        case rs_resource_state_indirect_argument:
        case rs_resource_state_copy_dest:
            layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case rs_resource_state_copy_source:
            layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            access_mask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case rs_resource_state_all_shader_resource:
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            access_mask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case rs_resource_state_vertex_and_constant_buffer:
            access_mask = VK_ACCESS_UNIFORM_READ_BIT;
            break;
        case rs_resource_state_common:
        case rs_resource_state_resolve_dest:
        case rs_resource_state_resolve_source:
        case rs_resource_state_raytracing_acceleration_structure:
        case rs_resource_state_shading_rate_source:
        case rs_resource_state_generic_read:
        case rs_resource_state_video_decode_read:
        case rs_resource_state_video_decode_write:
        case rs_resource_state_video_process_read:
        case rs_resource_state_video_process_write:
        case rs_resource_state_video_encode_read:
        case rs_resource_state_video_encode_write:
            assert(false);
            break;
    }
}




VkIndexType CastIndexType(ResourceFormat format) {
    switch(format){
        case ResourceFormat::rf_r16_uint: return VK_INDEX_TYPE_UINT16;
        case ResourceFormat::rf_r32_uint: return VK_INDEX_TYPE_UINT32;
        default: return VK_INDEX_TYPE_NONE_KHR;
    }
}

void CommandList::Reset() {
    m_pso = uint32_t(-1);
    m_root_sign = uint32_t(-1);
}

void CommandList::RSSetViewports(uint32_t num_viewports, const ViewPort* viewports) {
    VkViewport viewport_native = { 0, viewports->height, viewports->width, -viewports->height, 0, 1 };
    vkCmdSetViewport(m_command_list, 0, 1, &viewport_native);
}

void CommandList::RSSetScissorRects(uint32_t num_rects, const RectScissors* rects) {
    VkRect2D scissor = { {0, 0}, {rects->right, rects->bottom} };
    vkCmdSetScissor(m_command_list, 0, 1, &scissor);
}

void CommandList::SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) {
    assert(false);
}

void CommandList::SetComputeRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) {
    assert(false);
}

void CommandList::SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) {
    ((DynamicGpuHeap&)m_queue->GetGpuHeap()).StageDescriptorInTable(root_parameter_index, 0, buff);
}

void CommandList::SetComputeRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) {
    ((DynamicGpuHeap&)m_queue->GetGpuHeap()).StageDescriptorInTable(root_parameter_index, 0, buff);
}

void CommandList::SetIndexBuffer(const IndexVufferView* view) {
    HeapBuffer* buff = (HeapBuffer*)view->buffer_location.get();
    BufferMemAllocation buff_native = buff->GetBufferInfo();
    vkCmdBindIndexBuffer(m_command_list, buff_native.buffer, buff_native.offset, CastIndexType(view->format));
}

void CommandList::SetPrimitiveTopology(PrimitiveTopology primirive_topology) {
    return;
}

void CommandList::DrawInstanced(uint32_t vertex_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location) {
    vkCmdDraw(m_command_list, vertex_per_instance, instance_count,start_vertex_location, start_instance_location);
}

void CommandList::DrawIndexedInstanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location) {
    vkCmdDrawIndexed(m_command_list, index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location);
}

void CommandList::SetDescriptorHeap(const IDynamicGpuHeap* dynamic_heap) {
    assert(false);
}

void CommandList::SetRenderTargets(const std::vector<IGpuResource*>& resources, IGpuResource* depth_stencil_descriptor) {
    if (m_current_renderpass) {
        vkCmdEndRenderPass(m_command_list);
        m_current_renderpass = nullptr;
    }

    VkGpuResource *  rt = (VkGpuResource*) ( !resources.empty() ? resources.front() : depth_stencil_descriptor);

	VkRenderPassBeginInfo passBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	passBeginInfo.renderPass = rt->GetRenderPass();
	passBeginInfo.framebuffer = rt->GetFrameBuffer();
	passBeginInfo.renderArea.extent.width = rt->GetResourceDesc().width;
	passBeginInfo.renderArea.extent.height = rt->GetResourceDesc().height;

	vkCmdBeginRenderPass(m_command_list, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_current_renderpass = rt->GetRenderPass();
}

void CommandList::ClearRenderTargetView(IGpuResource* res, const float color[4], uint32_t num_rects, const RectScissors* rect) {
    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    // create a VkImageSubresourceRange to specify the image subresource to be cleared
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    if (auto buf = res->GetBuffer().lock()) {
        HeapBuffer* buff = (HeapBuffer*)buf.get();
        vkCmdClearColorImage(m_command_list, buff->GetImageInfo().image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);
    }
}

void CommandList::ClearRenderTargetView(IGpuResource& res, const float color[4], uint32_t num_rects, const RectScissors* rect) {
    ClearRenderTargetView(&res, color, num_rects, rect);
}

void CommandList::ClearDepthStencilView(IGpuResource* res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) {
    VkClearDepthStencilValue clearValue = {};
    clearValue.depth = depth; // set depth value to 1.0
    clearValue.stencil = stencil; // set stencil value to 0

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; // clear both depth and stencil aspects
    subresourceRange.baseMipLevel = 0; // start at base mip level
    subresourceRange.levelCount = 1; // clear only one mip level
    subresourceRange.baseArrayLayer = 0; // start at base array layer
    subresourceRange.layerCount = 1; // clear only one array layer

    if (auto buf = res->GetBuffer().lock()) {
        HeapBuffer* buff = (HeapBuffer*)buf.get();
        vkCmdClearDepthStencilImage(m_command_list, buff->GetImageInfo().image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &clearValue, 1, &subresourceRange);
    }
}

void CommandList::ClearDepthStencilView(IGpuResource& res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) {
    ClearDepthStencilView(&res, clear_flags, depth, stencil, num_rects, rects);
}

void CommandList::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) {
    vkCmdDispatch(m_command_list, thread_group_count_x, thread_group_count_y, thread_group_count_z);
}

void CommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) {
    ((DynamicGpuHeap&)m_queue->GetGpuHeap()).StageDescriptorInTable(root_parameter_index, 0, buff);
}

void CommandList::ResourceBarrier(std::shared_ptr<IGpuResource>& res, uint32_t to) {
    ResourceBarrier(*res, to);
}

void CommandList::ResourceBarrier(IGpuResource& res, uint32_t to) {
    if (auto buf = res.GetBuffer().lock()) {
        HeapBuffer* buff = (HeapBuffer*)buf.get();

        if (buff->GetVkType() == HeapBuffer::BufferResourceType::rt_texture) {
            VkImageMemoryBarrier barrier = {};
            ConvertResourceState(ResourceState(to), barrier.newLayout, barrier.dstAccessMask);
            ConvertResourceState(ResourceState(res.GetState()), barrier.oldLayout, barrier.srcAccessMask);
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = buff->GetImageInfo().image; // VkImage object
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Or VK_IMAGE_ASPECT_DEPTH_BIT for depth/stencil images
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

            vkCmdPipelineBarrier(
                m_command_list,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Or other shader stages
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }
        else {
            VkBufferMemoryBarrier bufferBarrier = {};
            VkImageLayout lay;
            ConvertResourceState(ResourceState(to), lay, bufferBarrier.dstAccessMask);
            ConvertResourceState(ResourceState(res.GetState()), lay, bufferBarrier.srcAccessMask);
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.pNext = nullptr;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.buffer = buff->GetBufferInfo().buffer;
            bufferBarrier.offset = 0;
            bufferBarrier.size = VK_WHOLE_SIZE;

            vkCmdPipelineBarrier(
                m_command_list,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                0,
                0, nullptr,
                1, &bufferBarrier,
                0, nullptr);
        }
        res.UpdateState(ResourceState(to));
    }
}

void CommandList::ResourceBarrier(std::vector<std::shared_ptr<IGpuResource>>& res, uint32_t to) {
    for (uint32_t i = 0; i < res.size(); i++) {
        ResourceBarrier(res[i], to);
    }
}

void CommandList::SetPSO(uint32_t id) {
    vkCmdBindPipeline(m_command_list, VK_PIPELINE_BIND_POINT_GRAPHICS, ((Techniques::TechniqueVk*)(gBackend->GetTechniqueById(id)))->pipeline);
}
