#include "FreeCamera.h"

FreeCamera::FreeCamera(float fov, float near, float far, float ascpect_ratio) :
    m_fov(fov),
    m_near(near),
    m_far(far)
{
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fov), ascpect_ratio, m_near, m_far);
    DirectX::XMStoreFloat4x4(&m_projection, projectionMatrix);

    DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3Normalize((DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&m_up), DirectX::XMLoadFloat3(&m_dir)))));
}

void FreeCamera::Move(const DirectX::XMFLOAT3 &pos){
    m_pos = pos;
}

void FreeCamera::Rotate(const DirectX::XMFLOAT3 &dir){
    DirectX::XMFLOAT3 dir_local = dir;
    DirectX::XMStoreFloat3(&m_dir, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir_local)));
}

void FreeCamera::Move(const DirectX::XMVECTOR &pos){
    DirectX::XMStoreFloat3(&m_pos, pos);
}

void FreeCamera::Rotate(const DirectX::XMVECTOR &dir){
    DirectX::XMStoreFloat3(&m_dir, DirectX::XMVector3Normalize(dir));
}

void FreeCamera::Update(float dt){
    DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3Normalize((DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&m_up), DirectX::XMLoadFloat3(&m_dir)))));
    const DirectX::XMVECTOR eyePosition = DirectX::XMLoadFloat3(&m_pos);
    const DirectX::XMVECTOR eyeDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_dir));
    const DirectX::XMVECTOR upDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_up));
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookToLH(eyePosition, eyeDirection, upDirection);
    DirectX::XMStoreFloat4x4(&m_view, viewMatrix);
}

   