#pragma once

#include <wrl.h>
using Microsoft::WRL::ComPtr;

struct IDXGIFactory4;
struct IDXGIAdapter1;
struct ID3D12Device2;

class DxDevice {
public:
	void OnInit(IDXGIFactory4* pFactory);
	ComPtr<ID3D12Device2>& GetNativeObject() { return m_device; }
private:
	void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

	ComPtr<ID3D12Device2> m_device;
};