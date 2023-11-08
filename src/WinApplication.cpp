#include "WinApplication.h"
#include "Frontend.h"
#include "defines.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

Frontend::KeyboardButton CastKey(uint8_t key) {
    Frontend::KeyboardButton btn{ Frontend::KeyboardButton::kb_none };
    switch (key) {
    case 'W':
        btn = Frontend::KeyboardButton::kb_w;
        break;
    case 'A':
        btn = Frontend::KeyboardButton::kb_a;
        break;
    case 'S':
        btn = Frontend::KeyboardButton::kb_s;
        break;
    case 'D':
        btn = Frontend::KeyboardButton::kb_d;
        break;
    case VK_OEM_3:
        btn = Frontend::KeyboardButton::kb_tilda;
        break;
    default:
        break;
    }

    return btn;
}

WinApplication::WinApplication(uint32_t width, uint32_t height, const std::wstring& window_name) :
    m_frontend(std::make_unique<Frontend>(width, height, window_name)),
    m_hwnd(nullptr),
    m_dx12(false),
    m_vk(false)
{
}

int WinApplication::Run(HINSTANCE hInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_frontend->GetWidth()), static_cast<LONG>(m_frontend->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        m_frontend->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        m_frontend.get());

    // Initialize the sample. OnInit is defined in each child-implementation of DXApp.
    WindowHandler w_hndl;
    w_hndl.ptr = (uint64_t)&m_hwnd;
    w_hndl.instance = (uint64_t)&hInstance;
    std::filesystem::path root_dir;

    // find absolute path
    wchar_t executablePath[MAX_PATH];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule)
    {
        GetModuleFileName(hModule, executablePath, sizeof(wchar_t) * MAX_PATH);
        std::filesystem::path p = std::filesystem::path(executablePath);
        root_dir = p.parent_path().parent_path().parent_path();
    }

    assert(m_dx12 ^ m_vk);
    Frontend::BackendType bk_type = m_dx12 ? Frontend::BackendType::bt_dx12 : Frontend::BackendType::bt_vk;

    m_frontend->OnInit(w_hndl, root_dir, bk_type);

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT && !m_frontend->ShouldClose())
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    m_frontend->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Helper function for parsing any supplied command line args.
void WinApplication::ParseCommandLineArgs(wchar_t *argv[], int argc) {
    for (int i = 1; i < argc; ++i) {
        if (_wcsnicmp(argv[i], L"-dx12", wcslen(argv[i])) == 0) {
            m_dx12 = true;
        } else if (_wcsnicmp(argv[i], L"-vk", wcslen(argv[i])) == 0) {
            m_vk = true;
        }
    }
}

// Main message handler for the sample.
LRESULT CALLBACK WinApplication::WindowProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{    
    Frontend* frontend = reinterpret_cast<Frontend*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    
    if (frontend) {
        ImguiWindowData data;
        data.hWnd.ptr = (uint64_t)&hWnd;
        data.msg = message;
        data.wParam = (uint64_t)wParam;
        data.lParam = (uint64_t)lParam;

        if(frontend->PassImguiWndProc(data))
            return true;
    }
        

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the frontend* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;
    
    case WM_RBUTTONDOWN:
        {
        frontend->OnMouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
		return 0;
	case WM_MOUSEMOVE:
        {
        Frontend::MouseButton btn{ Frontend::MouseButton::m_undefined };
        if ((wParam & MK_RBUTTON) != 0) {
            btn = Frontend::MouseButton::mb_right;
        }
        else if ((wParam & MK_LBUTTON) != 0) {
            btn = Frontend::MouseButton::mb_left;
        }
        else if ((wParam & MK_MBUTTON) != 0) {
            btn = Frontend::MouseButton::mb_middle;
        }

        frontend->OnMouseMoved(btn, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	    return 0;
        }
    case WM_KEYDOWN:
        if (frontend)
        {
            frontend->OnKeyDown(CastKey((uint8_t)wParam));
        }
        return 0;

    case WM_KEYUP:
        if (frontend)
        {
            frontend->OnKeyUp(CastKey((uint8_t)wParam));
        }
        return 0;

    case WM_PAINT:
        if (frontend)
        {
            frontend->OnUpdate();
            frontend->OnRender();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
