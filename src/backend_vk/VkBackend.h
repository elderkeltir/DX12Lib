#pragma once

#include "IBackend.h"
#include "IDescriptorHeapCollection.h"
#include <volk.h>
#include <memory>
#include <vector>
#include <array>
#include <filesystem>

// TODO: resolve this shit with forward declaration
#include "VkMemoryHelper.h"
#include "DynamicGpuHeap.h"

//class VkMemoryHelper;
class VkDeviceW;
class DescriptorHeapCollection;
class SwapChain;
class ShaderManager;
class Techniques;

class VkBackend : public IBackend {
public:
    ~VkBackend();
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
    uint32_t GetFrameCount() const override { return FramesCount; }
	IImguiHelper* GetUI() override;
	bool PassImguiWndProc(const ImguiWindowData& data) override;
	bool ShouldClose() override;
	void CreateFrameBuffer(std::vector<IGpuResource*> &rts, IGpuResource * depth, uint32_t tech_id) override;

	VkSampler GetSamplerById(uint32_t id) const;

	const std::filesystem::path& GetRootDir() const {
		return m_root_dir;
	}

	VkInstance GetInstance() {
		return m_instance;
	}

	VkMemoryHelper* GetMemoryHelper() {
		return m_memory_helper.get();
	}

	VkDeviceW* GetDevice() {
		return m_device.get();
	}

	ShaderManager* GetShaderManager() { return m_shader_mgr.get(); }

	IDescriptorHeapCollection* GetDescriptorHeapCollection() { return (IDescriptorHeapCollection*)m_descriptor_heap_collection.get(); }

    SwapChain* GetSwapChain() {
        return m_swap_chain.get();
    }

private:
    void CreateInstance(const std::vector<const char*> &extensions);
	static const uint32_t FramesCount = 2;
	static constexpr uint32_t GfxQueueCmdList_num = 6;
	static constexpr uint32_t ComputeQueueCmdList_num = 6;

	uint32_t m_frameIndex{0};

    ViewPort m_viewport;
    RectScissors m_scissorRect;

	VkInstance m_instance;
	std::unique_ptr<VkDeviceW> m_device;
	std::unique_ptr<VkMemoryHelper> m_memory_helper;
	std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
	std::unique_ptr<SwapChain> m_swap_chain;
	std::shared_ptr<ICommandQueue> m_commandQueueGfx;
	std::shared_ptr<ICommandQueue> m_commandQueueCompute;

	std::array<std::unique_ptr<IFence>, FramesCount> m_cpu_fences;
	std::unique_ptr<IFence> m_interqueue_fence;

	std::unique_ptr<logger> m_logger;
    //std::unique_ptr<IImguiHelper> m_gui;
	std::unique_ptr<Techniques> m_techniques;
	std::unique_ptr<ShaderManager> m_shader_mgr;

    uint32_t m_render_mode{ 0 };
    bool m_rebuild_shaders{ false };

    bool m_should_close{ false };

	std::filesystem::path m_root_dir;
};
