#pragma once

#include <wrl.h>
#include "CommandList.h"

using Microsoft::WRL::ComPtr;
struct ID3D12Device2;
struct ID3D12Fence;

class Fence {
public:
	void Initialize(const ComPtr<ID3D12Device2> &device, uint32_t val);
	ComPtr<ID3D12Fence>& GetFence() {
		return m_fence;
	}
private:
	ComPtr<ID3D12Fence> m_fence;
};