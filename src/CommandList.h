#pragma once

#include <directx/d3dx12.h>
#include "defines.h"

using Microsoft::WRL::ComPtr;

class CommandQueue;
struct IndexVufferView;
class GpuResource;

class CommandList {
	friend class CommandQueue;
public:
	void Reset();
	void RSSetViewports(uint32_t num_viewports, const ViewPort* viewports);
	void RSSetScissorRects(uint32_t num_rects, const RectScissors* rects);
	void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor);
	void SetComputeRootDescriptorTable(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor);
	void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location);
	void SetComputeRootConstantBufferView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location);
	void IASetIndexBuffer(const IndexVufferView* view);
	void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primirive_topology);
	void DrawInstanced(uint32_t vertex_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location);
	void DrawIndexedInstanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location);
	void SetDescriptorHeaps(uint32_t num_descriptor_heaps, ID3D12DescriptorHeap* const* descriptor_heap);
	void OMSetRenderTargets(uint32_t num_rt_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* render_target_descriptors, bool rt_single_descriptor_for_range, const D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil_descriptor);
	void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float color[4], uint32_t num_rects, const RectScissors* rect);
	void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsv, D3D12_CLEAR_FLAGS clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects);
	void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z);
	void SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS buffer_location);

	void ResourceBarrier(std::shared_ptr<GpuResource>& res, uint32_t to);
	void ResourceBarrier(GpuResource& res, uint32_t to);
	void ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, uint32_t to);
	void SetPSO(uint32_t id);
	void SetRootSign(uint32_t id, bool gfx = true);
	uint32_t GetPSO() const { return m_pso; }
	uint32_t GetRootSign() const { return m_root_sign; }

	ComPtr<ID3D12GraphicsCommandList6>& GetRawCommandList() { return m_command_list; }
	CommandQueue* GetQueue() { return m_queue; }
private:
	ComPtr<ID3D12GraphicsCommandList6> m_command_list;
	CommandQueue* m_queue{ nullptr };

	uint32_t m_pso{ uint32_t(-1) };
	uint32_t m_root_sign{ uint32_t(-1) };

	CommandListType m_type;
};