#pragma once

#include <string>

#include <wrl.h> // remove all these dx12 stuff
using Microsoft::WRL::ComPtr;
struct ID3D12Device2;

class RenderQuad;

class IImguiHelper {
public:
	enum class CaptureInput_type { cit_keyboard };

	virtual void Initialize(ComPtr<ID3D12Device2>& device, uint32_t frames_num) = 0;
	virtual void Destroy() = 0;
	virtual void Render(uint32_t frame_id) = 0;
	virtual bool WantCapture(CaptureInput_type type) const = 0;
	virtual void ShowConsole() = 0;
	virtual void AddToConsoleLog(const std::string& line) = 0;

	virtual RenderQuad* GetGuiQuad() = 0;
};