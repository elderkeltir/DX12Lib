#include "FreeCamera.h"

FreeCamera::FreeCamera(float fov, float near, float far, float ascpect_ratio) :
    m_up(0.f, 1.f, 0.f, 0),
    m_fov(fov),
    m_near(near),
    m_far(far)
{
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fov), ascpect_ratio, m_near, m_far);
    DirectX::XMStoreFloat4x4(&m_projection, projectionMatrix);
}

void FreeCamera::Move(const DirectX::XMFLOAT4 &pos){
    m_pos = pos;
}

void FreeCamera::Rotate(const DirectX::XMFLOAT4 &dir){
    m_dir = dir;
}

void FreeCamera::Update(float dt){
    const DirectX::XMVECTOR eyePosition = DirectX::XMLoadFloat4(&m_pos);
    const DirectX::XMVECTOR eyeDirection = DirectX::XMLoadFloat4(&m_dir);
    const DirectX::XMVECTOR upDirection = DirectX::XMLoadFloat4(&m_up);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookToLH(eyePosition, eyeDirection, upDirection);
    DirectX::XMStoreFloat4x4(&m_view, viewMatrix);
}

   