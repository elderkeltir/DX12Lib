#include "CommandList.h"

void CommandList::RSSetViewports(uint32_t num_viewports, const D3D12_VIEWPORT* viewports)
{
	m_command_list->RSSetViewports(num_viewports, viewports);
}

void CommandList::RSSetScissorRects(uint32_t num_rects, const D3D12_RECT* rects)
{
	m_command_list->RSSetScissorRects(num_rects, rects);
}

void CommandList::SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor)
{
	m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
}

void CommandList::SetComputeRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor)
{
	m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
}

void CommandList::SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
{
	m_command_list->SetGraphicsRootConstantBufferView(root_parameter_index, buffer_location);
}

void CommandList::SetComputeRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
{
	m_command_list->SetComputeRootConstantBufferView(root_parameter_index, buffer_location);
}

void CommandList::IASetVertexBuffers(uint32_t start_slot, uint32_t num_views, const D3D12_VERTEX_BUFFER_VIEW* view)
{
	m_command_list->IASetVertexBuffers(start_slot, num_views, view);
}

void CommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view)
{
	m_command_list->IASetIndexBuffer(view);
}

void CommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primirive_topology)
{
	m_command_list->IASetPrimitiveTopology(primirive_topology);
}

void CommandList::DrawIndexedInstanced(uint32_t index_count_per_index, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location)
{
	m_command_list->DrawIndexedInstanced(index_count_per_index, instance_count, start_index_location, base_vertex_location, start_instance_location);
}

void CommandList::SetDescriptorHeaps(uint32_t num_descriptor_heaps, ID3D12DescriptorHeap* const* descriptor_heap)
{
	m_command_list->SetDescriptorHeaps(num_descriptor_heaps, descriptor_heap);
}

void CommandList::OMSetRenderTargets(uint32_t num_rt_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* render_target_descriptors, bool rt_single_descriptor_for_range, const D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil_descriptor)
{
	m_command_list->OMSetRenderTargets(num_rt_descriptors, render_target_descriptors, rt_single_descriptor_for_range, depth_stencil_descriptor);
}

void CommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float color[4], uint32_t num_rects, const D3D12_RECT* rect)
{
	m_command_list->ClearRenderTargetView(rtv, color, num_rects, rect);
}

void CommandList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsv, D3D12_CLEAR_FLAGS clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const D3D12_RECT* rects)
{
	m_command_list->ClearDepthStencilView(dsv, clear_flags, depth, stencil, num_rects, rects);
}

void CommandList::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z)
{
	m_command_list->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}

void CommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
{
	m_command_list->SetGraphicsRootShaderResourceView(root_parameter_index, buffer_location);
}
