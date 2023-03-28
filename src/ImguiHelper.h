#pragma once

#include <directx/d3dx12.h>
#include <memory>

using Microsoft::WRL::ComPtr;

class CommandQueue;
class RenderQuad;
class AppConsole;

class ImguiHelper {
public:
	enum class CaptureInput_type { cit_keyboard };
	ImguiHelper();
	void Initialize(ComPtr<ID3D12Device2> &device, uint32_t frames_num);
	void Destroy();
	void Render(uint32_t frame_id);
	bool WantCapture(CaptureInput_type type) const;
	void ShowConsole();
	void AddToConsoleLog(const std::string& line);

	RenderQuad* GetGuiQuad() {
		return m_rt.get();
	}
	~ImguiHelper();
private:
	//Dx12
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12DescriptorHeap> m_gpu_visible_heap;
	std::unique_ptr<CommandQueue> m_commandQueueGfx;
	std::unique_ptr<RenderQuad> m_rt;
	std::unique_ptr<AppConsole> m_console;
	uint32_t m_frames_num{ 2 };
	//
	bool my_tool_active = true;
	float my_color[4];


	bool show_demo_window = true;
	bool show_another_window = false;
};