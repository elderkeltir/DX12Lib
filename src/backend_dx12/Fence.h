#pragma once

#include <wrl.h>
#include "IFence.h"

using Microsoft::WRL::ComPtr;
struct ID3D12Fence;

class Fence : public IFence {
public:
	void Initialize(uint32_t val) override;
	ComPtr<ID3D12Fence>& GetFence() {
		return m_fence;
	}
	~Fence();
private:
	ComPtr<ID3D12Fence> m_fence;
};