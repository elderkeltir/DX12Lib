#include "stdafx.h"

#include "DXAppImplementation.h"
#include "Application.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    DXAppImplementation sample(1280, 720, L"D3D12 Hello Window");
    return Application::Run(&sample, hInstance, nCmdShow);
}
