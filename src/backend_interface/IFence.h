#pragma once

#include <cstdint>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
struct ID3D12Device2;

class IFence {
public:
	virtual void Initialize(const ComPtr<ID3D12Device2>& device, uint32_t val) = 0; // TODO: get rif of device
	virtual ~IFence() {}
};