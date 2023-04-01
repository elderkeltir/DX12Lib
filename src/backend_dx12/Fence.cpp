#include "Fence.h"
#include "dx12_helper.h"
#include <directx/d3dx12.h>

void Fence::Initialize(const ComPtr<ID3D12Device2>& device, uint32_t val)
{
	ThrowIfFailed(device->CreateFence(val, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_fence)));
}

Fence::~Fence() = default;