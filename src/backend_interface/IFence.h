#pragma once

#include <cstdint>

class IFence {
public:
	virtual void Initialize(uint32_t val) = 0;
	virtual ~IFence() {}
};