#include "Fence.h"
#include "dx12_helper.h"
#include <directx/d3dx12.h>
#include "DxBackend.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

void Fence::Initialize(uint32_t val)
{
	ComPtr<ID3D12Device2>& device = gBackend->GetDevice()->GetNativeObject();
	ThrowIfFailed(device->CreateFence(val, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_fence)));
}

Fence::~Fence() = default;