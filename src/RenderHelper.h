#pragma once

#include <cmath>

#define _STR(x) #x
#define STR(x) _STR(x)
#define TODO(x) __pragma(message("TODO: "_STR(x) "::" __FILE__ ":" STR(__LINE__)))
#define CHECK(x) __pragma(message("CHECK: "_STR(x) "::" __FILE__ ":" STR(__LINE__)))

inline uint32_t calc_cb_size(uint32_t size) {
    return (size + 255) & ~255; // TODO: D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT  and fix for vulkan
}

inline bool cmpf(float A, float B, float epsilon = 0.00001f)
{
    return (fabs(A - B) < epsilon);
}

static float rand_fp_unorm()
{
	return (float)(rand()) / (float)RAND_MAX;
}

static float rand_fp(float a, float b)
{
	return a + rand_fp_unorm() * (b - a);
}

inline float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}



// PIXSetMarker(m_commandList.Get(), PIX_COLOR(120, 120, 120), L"UpdateBufferResource for index buffer");
//#define BEGIN_EVENT(cmd_list, name) PIXBeginEvent(cmd_list.Get(), PIX_COLOR(55, 120, 55), name)
//#define END_EVENT(cmd_list) PIXEndEvent(cmd_list.Get())
// PIXBeginEvent(m_commandList.Get(), PIX_COLOR(55, 120, 55), L"LoadTechnique");
// PIXEndEvent(m_commandList.Get());
// PIXCaptureParameters cap_params{};
// cap_params.GpuCaptureParameters.FileName = L"capture.wpix";
// PIXBeginCapture(PIX_CAPTURE_GPU , &cap_params);
// PIXEndCapture(FALSE);
