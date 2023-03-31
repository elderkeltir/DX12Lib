#include "Application.h"
#include "DXHelper.h"
#include "DXApp.h"

using Microsoft::WRL::ComPtr;

DXApp::DXApp(uint32_t width, uint32_t height, std::wstring name) : m_width(width),
                                                           m_height(height),
                                                           m_title(name),
                                                           m_useWarpDevice(false)
{
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DXApp::~DXApp()
{
}

// Helper function for setting the window's title text.
void DXApp::SetCustomWindowText(LPCWSTR text)
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Application::GetHwnd(), windowText.c_str());
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_ void DXApp::ParseCommandLineArgs(wchar_t *argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            m_useWarpDevice = true;
            m_title = m_title + L" (WARP)";
        }
    }
}
