#pragma once

#include "IImguiHelper.h"
#include "IDynamicGpuHeap.h"
#include <directx/d3dx12.h>
#include <memory>

#include <wrl.h>                // COM helpers.
using Microsoft::WRL::ComPtr;

class ICommandQueue;
class RenderQuad;
class AppConsole;


class ImguiHelper : public IImguiHelper {
public:
	ImguiHelper();

	void Initialize(ComPtr<ID3D12Device2> &device, uint32_t frames_num) override;
	void Destroy() override;
	void Render(uint32_t frame_id) override;
	bool WantCapture(CaptureInput_type type) const override;
	void ShowConsole() override;
	void AddToConsoleLog(const std::string& line) override;

	RenderQuad* GetGuiQuad() override {
		return m_rt.get();
	}
	~ImguiHelper();
private:
	//Dx12
	ComPtr<ID3D12Device2> m_device;
	std::unique_ptr<IDynamicGpuHeap> m_gpu_visible_heap;
	std::unique_ptr<ICommandQueue> m_commandQueueGfx;
	std::unique_ptr<RenderQuad> m_rt;
	std::unique_ptr<AppConsole> m_console;
	uint32_t m_frames_num{ 2 };
	//
	bool my_tool_active = true;
	float my_color[4];


	bool show_demo_window = true;
	bool show_another_window = false;
};