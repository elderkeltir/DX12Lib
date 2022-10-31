#pragma once


#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <directx/d3dx12.h>
#include <string>

//
//#include "Application.h"

class DXApp
{
public:
    DXApp(uint32_t width, uint32_t height, std::wstring name);
    virtual ~DXApp();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;

    // Samples override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/)   {}
    virtual void OnKeyUp(UINT8 /*key*/)     {}

    // Accessors.
    uint32_t GetWidth() const       { return m_width; }
    uint32_t GetHeight() const      { return m_height; }
    const WCHAR* GetTitle() const   { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    void SetCustomWindowText(LPCWSTR text);

    // Viewport dimensions.
    uint32_t m_width;
    uint32_t m_height;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

private:
    // Window title.
    std::wstring m_title;
};