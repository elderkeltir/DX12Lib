#pragma once

#include <DirectXMath.h>

class FreeCamera {
public:
    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &dir);
private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_dir;
    DirectX::XMFLOAT3 m_up;
    DirectX::XMFLOAT3 m_right;
};