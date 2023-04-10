#include "CommandList.h"
#include "vk_helper.h"
#include "IGpuResource.h"
#include "HeapBuffer.h"

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
    assert(false);
}

void CommandList::SetComputeRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) {
    assert(false);
}

void CommandList::SetIndexBuffer(const IndexVufferView* view) {
    // get buffer info
    HeapBuffer* buff = (HeapBuffer*)view->buffer_location.get();
    VkDeviceSize size, offset;
    VkBuffer buff_native = buff->GetBufferInfo(size, offset);
    vkCmdBindIndexBuffer(m_command_list, buff_native, offset, CastIndexType(view->format));
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
    assert(false); // TODO: maybe something to mix with renderpass, begin render path etx
}

void CommandList::ClearRenderTargetView(IGpuResource* res, const float color[4], uint32_t num_rects, const RectScissors* rect) {
    assert(false); // TODO: might be necessary vkCmdBeginRenderPass OR vkCmdClearColorImage()
}

void CommandList::ClearRenderTargetView(IGpuResource& res, const float color[4], uint32_t num_rects, const RectScissors* rect) {
    assert(false); // TODO: might be necessary vkCmdBeginRenderPass
}

void CommandList::ClearDepthStencilView(IGpuResource* res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) {
    assert(false); // TODO: maybe vkCmdClearColorImage()
}

void CommandList::ClearDepthStencilView(IGpuResource& res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) {
    assert(false); // TODO: maybe vkCmdClearColorImage()
}

void CommandList::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) {
    vkCmdDispatch(m_command_list, thread_group_count_x, thread_group_count_y, thread_group_count_z);
}

void CommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) {
    assert(false); // TODO: ???
}

void CommandList::ResourceBarrier(std::shared_ptr<IGpuResource>& res, uint32_t to) {
    return; // TODO: there is no any kekw
}

void CommandList::ResourceBarrier(IGpuResource& res, uint32_t to) {
    return; // TODO: there is no any kekw
}

void CommandList::ResourceBarrier(std::vector<std::shared_ptr<IGpuResource>>& res, uint32_t to) {
    return; // TODO: there is no any kekw
}

void CommandList::SetPSO(uint32_t id) {
    assert(false);
}