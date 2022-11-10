#pragma once
#include <DirectXMath.h>

struct Vertex0
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Color;
};
struct Vertex1
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Tangents;
    DirectX::XMFLOAT3 Bitangents;
    DirectX::XMFLOAT2 TextCoord;
};

inline uint32_t GetSizeByVertexType(uint32_t id) {
    switch (id) {
        case 0:
        return sizeof(Vertex0);
        break;
    case 1:
        return sizeof(Vertex1);
        break;
    default:
        assert(false);
    }

    return 0;
}