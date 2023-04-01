

 
// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif
 
#if defined(max)
#undef max
#endif
 
#ifdef WIN32
// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif
 
// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include "WinApplication.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WinApplication app(1280, 720, L"DX12Lib");
    return app.Run(hInstance, nCmdShow);
}
#else
int main() {
    return error;
}
#endif // WIN32
