#pragma once

#include <string>

class IGpuResource;

class IImguiHelper {
public:
	enum class CaptureInput_type { cit_keyboard };

	virtual void Initialize(uint32_t frames_num) = 0;
	virtual void Destroy() = 0;
	virtual void Render(uint32_t frame_id) = 0;
	virtual bool WantCapture(CaptureInput_type type) const = 0;
	virtual void ShowConsole() = 0;
	virtual void AddToConsoleLog(const std::string& line) = 0;

	virtual IGpuResource* GetGuiQuad(uint32_t frame_id) = 0;
	virtual ~IImguiHelper() = default;
};