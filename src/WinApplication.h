#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

#include "Frontend.h"
#include <memory>

class Frontend;

class WinApplication
{
public:
    WinApplication(uint32_t width, uint32_t height, const std::wstring& window_name);
    int Run(HINSTANCE hInstance, LPSTR pCmdLine, int nCmdShow); // TODO: rewrite top lvl of the application. unnecessary depends

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

private:
    void ParseCommandLineArgs(wchar_t* argv[], int argc);
    std::unique_ptr<Frontend> m_frontend;
    HWND m_hwnd;

    bool m_dx12;
    bool m_vk;
};