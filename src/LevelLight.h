#pragma once

#include <DirectXMath.h>

constexpr uint32_t LightsNum = 16u;

// size = 48
struct LevelLight {
    LevelLight() : type(LightType::lt_none) {}
    
    DirectX::XMFLOAT3 pos;
    enum class LightType { lt_none = 0, lt_ambient, lt_direct, lt_point, lt_spot } type;
    DirectX::XMFLOAT3 dir;
    uint32_t id;
    DirectX::XMFLOAT3 color;
    uint32_t padding;
};