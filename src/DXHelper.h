#pragma once

#include <stdexcept>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#define _STR(x) #x
#define STR(x) _STR(x)
#define TODO(x) __pragma(message("TODO: "_STR(x) "::" __FILE__ ":" STR(__LINE__)))
#define CHECK(x) __pragma(message("CHECK: "_STR(x) "::" __FILE__ ":" STR(__LINE__)))

inline uint32_t calc_cb_size(uint32_t size) {
    return (size + 255) & ~255;
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

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<uint32_t>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
inline void SetName(ComPtr<ID3D12Object> pObject, LPCWSTR name)
{
    pObject.Get()->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, uint32_t index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        pObject->SetName(fullName);
    }
}
#else
#define SetName(ptr, name)
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, uint32_t)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

inline uint32_t CalculateConstantBufferByteSize(uint32_t byteSize)
{
    // Constant buffer size is required to be aligned.
    return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}



// PIXSetMarker(m_commandList.Get(), PIX_COLOR(120, 120, 120), L"UpdateBufferResource for index buffer");
#define BEGIN_EVENT(cmd_list, name) PIXBeginEvent(cmd_list.Get(), PIX_COLOR(55, 120, 55), name)
#define END_EVENT(cmd_list) PIXEndEvent(cmd_list.Get())
// PIXBeginEvent(m_commandList.Get(), PIX_COLOR(55, 120, 55), L"LoadTechnique");
// PIXEndEvent(m_commandList.Get());
// PIXCaptureParameters cap_params{};
// cap_params.GpuCaptureParameters.FileName = L"capture.wpix";
// PIXBeginCapture(PIX_CAPTURE_GPU , &cap_params);
// PIXEndCapture(FALSE);