#pragma once

#include "IBackend.h"
#include "IDescriptorHeapCollection.h"
#include <volk.h>
#include <memory>

class VkMemoryHelper;
class VkDeviceW;
class DescriptorHeapCollection;
class SwapChain;

class VkBackend : public IBackend {
public:
	void OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path) override;
	uint32_t GetCurrentBackBufferIndex() const override;
	IGpuResource& GetCurrentBackBuffer() override;
	IGpuResource* GetDepthBuffer() override;
	void Present() override;
	void OnResizeWindow() override;
	std::shared_ptr<ICommandQueue>& GetQueue(ICommandQueue::QueueType type) override;
	logger* GetLogger() override;
	void SyncWithCPU() override;
	void SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to) override;
	ICommandList* InitCmdList() override;
	void ChechUpdatedShader() override;
	void RenderUI() override;
	void DebugSectionBegin(ICommandList* cmd_list, const std::string & name) override;
	void DebugSectionEnd(ICommandList* cmd_list) override;
	const ITechniques::Technique* GetTechniqueById(uint32_t id) const override;
	const IRootSignature* GetRootSignById(uint32_t id) override;
	uint32_t GetRenderMode() const override;
	uint32_t GetFrameCount() const override;
	IImguiHelper* GetUI() override;
	bool PassImguiWndProc(const ImguiWindowData& data) override;
	bool ShouldClose() override;

	VkInstance GetInstance() {
		return m_instance;
	}

	VkMemoryHelper* GetMemoryHelper() {
		return m_memory_helper.get();
	}

	VkDeviceW* GetDevice() {
		return m_device.get();
	}

	IDescriptorHeapCollection* GetDescriptorHeapCollection() { return (IDescriptorHeapCollection*)m_descriptor_heap_collection.get(); }

private:
	static const uint32_t FramesCount = 2;
	static constexpr uint32_t GfxQueueCmdList_num = 6;
	static constexpr uint32_t ComputeQueueCmdList_num = 6;

	uint32_t m_frameIndex{0};

	VkInstance m_instance;
	std::unique_ptr<VkDeviceW> m_device;
	std::unique_ptr<VkMemoryHelper> m_memory_helper;
	std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
	std::unique_ptr<SwapChain> m_swap_chain;
	std::shared_ptr<ICommandQueue> m_commandQueueGfx;
	std::shared_ptr<ICommandQueue> m_commandQueueCompute;

	std::array<IFence*, FramesCount> m_cpu_fences;
	std::unique_ptr<IFence> m_interqueue_fence;
};
