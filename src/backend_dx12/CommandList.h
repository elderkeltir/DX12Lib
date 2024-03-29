#pragma once

#include "ICommandList.h"
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include "IGpuResource.h"

#if defined(USE_NSIGHT_AFTERMATH)
#include "NsightAftermathHelpers.h"
#endif

struct ID3D12GraphicsCommandList6;
class ICommandQueue;
struct IndexVufferView;
class IHeapBuffer;
class IDynamicGpuHeap;

class CommandList : public ICommandList{
	friend class CommandQueue;
public:
	void Reset() override;
	void RSSetViewports(uint32_t num_viewports, const ViewPort* viewports) override;
	void RSSetScissorRects(uint32_t num_rects, const RectScissors* rects) override;
	void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) override;
	void SetComputeRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor) override;
	void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer> &buff) override;
	void SetComputeRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer> &buff) override;
	void SetIndexBuffer(const IndexVufferView* view) override;
	void SetPrimitiveTopology(PrimitiveTopology primirive_topology) override;
	void DrawInstanced(uint32_t vertex_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location) override;
	void DrawIndexedInstanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location) override;
	void SetDescriptorHeap(const IDynamicGpuHeap*dynamic_heap) override;
	void SetRenderTargets(const std::vector<IGpuResource*> &resources, IGpuResource* depth_stencil_descriptor) override;
	void ClearRenderTargetView(IGpuResource* res, const float color[4], uint32_t num_rects, const RectScissors* rect) override;
	void ClearRenderTargetView(IGpuResource& res, const float color[4], uint32_t num_rects, const RectScissors* rect) override {
		ClearRenderTargetView(&res, color, num_rects, rect);
	}
	void ClearDepthStencilView(IGpuResource* res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) override;
	void ClearDepthStencilView(IGpuResource& res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects) override {
		ClearDepthStencilView(&res, clear_flags, depth, stencil, num_rects, rects);
	}
	void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) override;
	void SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, const std::shared_ptr<IHeapBuffer> &buff) override;

	void ResourceBarrier(std::shared_ptr<IGpuResource>& res, uint32_t to) override;
	void ResourceBarrier(IGpuResource& res, uint32_t to) override;
	void ResourceBarrier(std::vector<std::shared_ptr<IGpuResource>>& res, uint32_t to) override;
	void SetPSO(uint32_t id) override;
	void SetRootSign(uint32_t id, bool gfx = true) override;
	uint32_t GetPSO() const  override { return m_pso; }
	uint32_t GetRootSign() const  override { return m_root_sign; }

	ComPtr<ID3D12GraphicsCommandList6>& GetRawCommandList() { return m_command_list; }
	ICommandQueue* GetQueue()  override { return m_queue; }
private:
	ComPtr<ID3D12GraphicsCommandList6> m_command_list;
	ICommandQueue* m_queue{ nullptr };

	uint32_t m_pso{ uint32_t(-1) };
	uint32_t m_root_sign{ uint32_t(-1) };

	CommandListType m_type;

#if defined(USE_NSIGHT_AFTERMATH)
	// Nsight Aftermath instrumentation
	GFSDK_Aftermath_ContextHandle m_hAftermathCommandListContext{ nullptr };
	void SetupNvAfterMath();
public:
	GFSDK_Aftermath_ContextHandle& GetNvAfterMathHndl() {
		return m_hAftermathCommandListContext;
	}
#endif
};