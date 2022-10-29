#pragma once

#include <DirectXMath.h>

class FreeCamera {
public:
    FreeCamera(float fov, float near, float far, float ascpect_ratio);
    void Move(const DirectX::XMFLOAT4 &pos);
    void Rotate(const DirectX::XMFLOAT4 &dir);
    void Update(float dt);

    const DirectX::XMFLOAT4X4& GetViewMx() const { return m_view; }
    const DirectX::XMFLOAT4X4& GetProjMx() const { return m_projection; }

private:
    DirectX::XMFLOAT4 m_pos;
    DirectX::XMFLOAT4 m_dir;
    DirectX::XMFLOAT4 m_up;
    DirectX::XMFLOAT4 m_right;
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_projection;

    float m_fov;
    float m_near;
    float m_far;
};