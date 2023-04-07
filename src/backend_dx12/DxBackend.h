#pragma once

#include "IBackend.h"
#include "ICommandQueue.h"
#include "defines.h"
#include "IFence.h"

#include <memory>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
struct IDXGIFactory4;
struct IDXGIFactory1;
struct IDXGIAdapter1;

class IGpuResource;
struct WindowHandler;
class DxDevice;
class SwapChain;
class CommandQueue;
class logger;
class IDescriptorHeapCollection;
class DescriptorHeapCollection;
class IImguiHelper;
class ShaderManager;

class DxBackend : public IBackend {
public:
	DxBackend() = default;
	void OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path) override;
	uint32_t GetCurrentBackBufferIndex() const override;
	IGpuResource& GetCurrentBackBuffer() override;
	IGpuResource* GetDepthBuffer() override;
	void Present() override;
	void OnResizeWindow() override;
	std::shared_ptr<ICommandQueue>& GetQueue(ICommandQueue::QueueType type) override {
		return (type == ICommandQueue::QueueType::qt_gfx ? m_commandQueueGfx : m_commandQueueCompute);
	}
	logger* GetLogger() override { return m_logger.get(); }
	void SyncWithCPU() override;
	void SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to) override;
	ICommandList* InitCmdList() override;
	void ChechUpdatedShader() override;
	void RenderUI() override;
	void DebugSectionBegin(ICommandList* cmd_list, const std::string& name) override;
	void DebugSectionEnd(ICommandList* cmd_list) override;
	const ITechniques::Technique* GetTechniqueById(uint32_t id) const override;
	const IRootSignature* GetRootSignById(uint32_t id) override;
	uint32_t GetRenderMode() const override { return m_render_mode; }
	uint32_t GetFrameCount() const override { return FramesCount; }
	IImguiHelper* GetUI() override { return m_gui.get(); }
	void RebuildShaders(std::optional<std::wstring> dbg_name = std::nullopt);
	void SetRenderMode(uint32_t mode) { m_render_mode = mode; }
	bool PassImguiWndProc(const ImguiWindowData& data) override;
	bool ShouldClose() override { return m_should_close; }

	const std::filesystem::path& GetRootDir() const {
		return m_root_dir;
	}
	ShaderManager* GetShaderManager() { return m_shader_mgr.get(); }
	SwapChain* GetSwapChain() { return m_swap_chain.get(); }
	IDescriptorHeapCollection* GetDescriptorHeapCollection() { return (IDescriptorHeapCollection*)m_descriptor_heap_collection.get(); }
	IImguiHelper* GetUiHelper() { return m_gui.get(); }
	DxDevice* GetDevice() { return m_device.get();  }
	void Close() { m_should_close = true; }
	virtual ~DxBackend();
private:
	static const uint32_t FramesCount = 2;
	static constexpr uint32_t GfxQueueCmdList_num = 6;
	static constexpr uint32_t ComputeQueueCmdList_num = 6;

	ComPtr<IDXGIFactory4> m_factory;
	std::unique_ptr<DxDevice> m_device;
	std::unique_ptr<SwapChain> m_swap_chain;
	std::shared_ptr<ICommandQueue> m_commandQueueGfx;
	std::shared_ptr<ICommandQueue> m_commandQueueCompute;

	ViewPort m_viewport;
	RectScissors m_scissorRect;

	std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
	std::unique_ptr<logger> m_logger;
	std::unique_ptr<IImguiHelper> m_gui;
	std::unique_ptr<ITechniques> m_techniques;
	std::unique_ptr<ShaderManager> m_shader_mgr;

	std::unique_ptr<IFence> m_fence_inter_queue;
	uint32_t m_fence_inter_queue_val{ 0 };

	uint32_t m_frameIndex{ 0 };
	uint32_t m_fenceValues[FramesCount]{ 0 };

	uint32_t m_render_mode{ 0 };
	bool m_rebuild_shaders{ false };

	bool m_should_close{ false };

	std::filesystem::path m_root_dir;
};
