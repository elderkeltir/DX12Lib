#pragma once

#include "IImguiHelper.h"
#include "IDynamicGpuHeap.h"

#include <memory>
#include <array>

#include <wrl.h>                // COM helpers.
using Microsoft::WRL::ComPtr;

class ICommandQueue;
class IGpuResource;
class AppConsole;
struct ID3D12Device2;


class ImguiHelper : public IImguiHelper {
public:
	ImguiHelper();

	void Initialize(uint32_t frames_num) override;
	void Destroy() override;
	void Render(uint32_t frame_id) override;
	bool WantCapture(CaptureInput_type type) const override;
	void ShowConsole() override;
	void AddToConsoleLog(const std::string& line) override;

	IGpuResource* GetGuiQuad(uint32_t frame_id) override {
		return m_rts[frame_id].get();
	}
	bool PassImguiWndProc(const ImguiWindowData& data);
	~ImguiHelper();
private:
	void CreateQuadTexture(uint32_t width, uint32_t height, ResourceFormat formats, uint32_t texture_nums);

	//Dx12
	ComPtr<ID3D12Device2> m_device;
	std::unique_ptr<IDynamicGpuHeap> m_gpu_visible_heap;
	std::unique_ptr<ICommandQueue> m_commandQueueGfx;
	std::array<std::unique_ptr<IGpuResource>, 2> m_rts;
	std::unique_ptr<AppConsole> m_console;
	uint32_t m_frames_num{ 2 };
	//
	bool my_tool_active = true;
	float my_color[4];

	bool m_is_initialized{ false };
	bool show_demo_window = true;
	bool show_another_window = false;
};