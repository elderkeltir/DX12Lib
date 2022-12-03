#pragma once

#include <DirectXMath.h>

class FreeCamera {
public:
    FreeCamera(float fov, float near, float far, float ascpect_ratio);
    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &dir);
    void Move(const DirectX::XMVECTOR &pos);
    void Rotate(const DirectX::XMVECTOR &dir);
    void Update(float dt);

    const DirectX::XMFLOAT4X4& GetViewMx() const { return m_view; }
    const DirectX::XMFLOAT4X4& GetProjMx() const { return m_projection; }

    const DirectX::XMFLOAT3& GetDirection() const { return m_dir; }
    const DirectX::XMFLOAT3& GetRightDirection() const { return m_right; }
    const DirectX::XMFLOAT3& GetUpDirection() const { return m_up; }
    const DirectX::XMFLOAT3& GetPosition() const { return m_pos; }

    float GetNearZ() const { return m_near; }
    float GetFarZ() const { return m_far; }

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_dir;
    const DirectX::XMFLOAT3 m_up{0, 1, 0};
    DirectX::XMFLOAT3 m_right;
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_projection;

    float m_fov;
    float m_near;
    float m_far;
};