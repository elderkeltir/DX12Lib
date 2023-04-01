#pragma once

#include <memory>
#include <vector>
#include "defines.h"
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class IHeapBuffer;
class IGpuResource;
class ICommandQueue;
class IDynamicGpuHeap;
struct IndexVufferView;
struct ID3D12GraphicsCommandList6;

class ICommandList {
	friend class ICommandQueue;
public:
	virtual void Reset() = 0;
	virtual void RSSetViewports(uint32_t num_viewports, const ViewPort* viewports) = 0;
	virtual void RSSetScissorRects(uint32_t num_rects, const RectScissors* rects) = 0;
	virtual void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) = 0;
	virtual void SetComputeRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) = 0;
	virtual void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) = 0;
	virtual void SetComputeRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) = 0;
	virtual void SetIndexBuffer(const IndexVufferView* view) = 0;
	virtual void SetPrimitiveTopology(PrimitiveTopology primirive_topology) = 0;
	virtual void DrawInstanced(uint32_t vertex_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location) = 0;
	virtual void DrawIndexedInstanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location) = 0;
	virtual void SetDescriptorHeap(const IDynamicGpuHeap& dynamic_heap) = 0;
	virtual void SetRenderTargets(const std::vector<IGpuResource*>& resources, IGpuResource* depth_stencil_descriptor) = 0;
	virtual void ClearRenderTargetView(IGpuResource* res, const float color[4], uint32_t num_rects, const RectScissors* rect) = 0;
	virtual void ClearRenderTargetView(IGpuResource& res, const float color[4], uint32_t num_rects, const RectScissors* rect) = 0;
	virtual void ClearDepthStencilView(IGpuResource* res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) = 0;
	virtual void ClearDepthStencilView(IGpuResource& res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) = 0;
	virtual void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) = 0;
	virtual void SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer>& buff) = 0;

	virtual void ResourceBarrier(std::shared_ptr<IGpuResource>& res, uint32_t to) = 0;
	virtual void ResourceBarrier(IGpuResource& res, uint32_t to) = 0;
	virtual void ResourceBarrier(std::vector<std::shared_ptr<IGpuResource>>& res, uint32_t to) = 0;
	virtual void SetPSO(uint32_t id) = 0;
	virtual void SetRootSign(uint32_t id, bool gfx = true) = 0;
	virtual uint32_t GetPSO() const = 0;
	virtual uint32_t GetRootSign() const = 0;

	virtual ComPtr<ID3D12GraphicsCommandList6>& GetRawCommandList() = 0;
	virtual ICommandQueue* GetQueue() = 0;
};