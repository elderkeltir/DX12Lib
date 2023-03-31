#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

#include <string>

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
    virtual void OnKeyDown(uint8_t key)   {}
    virtual void OnKeyUp(uint8_t key)     {}
    virtual void OnMouseButtonDown(int x, int y) {}
    virtual void OnMouseMoved(WPARAM btnState, int x, int y) {}

    // Accessors.
    uint32_t GetWidth() const       { return m_width; }
    uint32_t GetHeight() const      { return m_height; }
    const WCHAR* GetTitle() const   { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) wchar_t* argv[], int argc);

protected:
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