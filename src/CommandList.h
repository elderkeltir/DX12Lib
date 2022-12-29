#pragma once

#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

class GfxCommandQueue;

class CommandList {
	friend class GfxCommandQueue;
public:
	void RSSetViewports(uint32_t num_viewports, const D3D12_VIEWPORT* viewports);
	void RSSetScissorRects(uint32_t num_rects, const D3D12_RECT* rects);
	void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor);
	void SetComputeRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor);
	void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location);
	void SetComputeRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location);
	void IASetVertexBuffers(uint32_t start_slot, uint32_t num_views, const D3D12_VERTEX_BUFFER_VIEW* view); // TODO: remove when bindless vertex buffer implemented
	void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view);
	void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primirive_topology);
	void DrawIndexedInstanced(uint32_t index_count_per_index, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location);
	void SetDescriptorHeaps(uint32_t num_descriptor_heaps, ID3D12DescriptorHeap* const* descriptor_heap); // TODO: change second parameter to my own type?
	void OMSetRenderTargets(uint32_t num_rt_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* render_target_descriptors, bool rt_single_descriptor_for_range, const D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil_descriptor);
	void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float color[4], uint32_t num_rects, const D3D12_RECT* rect);
	void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsv, D3D12_CLEAR_FLAGS clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const D3D12_RECT* rects);
	void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z);

	ComPtr<ID3D12GraphicsCommandList6>& GetRawCommandList() { return m_command_list; }
	GfxCommandQueue* GetQueue() { return m_queue; }
private:
	ComPtr<ID3D12GraphicsCommandList6> m_command_list;
	GfxCommandQueue* m_queue{ nullptr };
};