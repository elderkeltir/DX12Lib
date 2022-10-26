#pragma once

#include <memory>
#include <DirectXMath.h>

class Transformations
{
public:
    Transformations();
    ~Transformations();
    DirectX::XMMATRIX GetModel();

    void Move(const DirectX::XMFLOAT3 &direction);
    void Rotate(const DirectX::XMFLOAT3 &rotation);
    void Scale(const DirectX::XMFLOAT3 &scale);

private:
    DirectX::XMFLOAT4X4 m_model;
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};