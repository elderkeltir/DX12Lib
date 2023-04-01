#pragma once

#include <cstdint>

class IRootSignature {
public:
	virtual uint32_t GetRSId() const = 0;
	virtual void SetRSId(uint32_t id) = 0;
};