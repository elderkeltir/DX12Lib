#pragma once

#include "DXApp.h"

class DXApp;

class Application
{
public:
    static int Run(DXApp* pSample, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }
    static void Close() { m_force_close = true; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
    static bool m_force_close;
};