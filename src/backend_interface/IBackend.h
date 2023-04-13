#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include "ICommandQueue.h"
#include "ITechniques.h"

class IGpuResource;
struct WindowHandler;
class logger;
class ICommandList;
class IRootSignature;
class IImguiHelper;
struct ImguiWindowData;

class IBackend {
public:
	virtual void OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path) = 0;
	virtual uint32_t GetCurrentBackBufferIndex() const = 0;
	virtual IGpuResource& GetCurrentBackBuffer() = 0;
	virtual IGpuResource* GetDepthBuffer() = 0;
	virtual void Present() = 0;
	virtual void OnResizeWindow() = 0;
	virtual std::shared_ptr<ICommandQueue>& GetQueue(ICommandQueue::QueueType type) = 0;
	virtual logger* GetLogger() = 0;
	virtual void SyncWithCPU() = 0;
	virtual void SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to) = 0;
	virtual ICommandList* InitCmdList() = 0;
	virtual void ChechUpdatedShader() = 0;
	virtual void RenderUI() = 0;
	virtual void DebugSectionBegin(ICommandList* cmd_list, const std::string & name) = 0;
	virtual void DebugSectionEnd(ICommandList* cmd_list) = 0;
	virtual const ITechniques::Technique* GetTechniqueById(uint32_t id) const = 0;
	virtual const IRootSignature* GetRootSignById(uint32_t id) = 0;
	virtual uint32_t GetRenderMode() const = 0;
	virtual uint32_t GetFrameCount() const = 0;
	virtual IImguiHelper* GetUI() = 0;
	virtual bool PassImguiWndProc(const ImguiWindowData& data) = 0;
	virtual bool ShouldClose() = 0;
	virtual void CreateFrameBuffer(std::vector<IGpuResource*> &rts, IGpuResource * depth, uint32_t tech_id) = 0;
	virtual ~IBackend() = default;
};