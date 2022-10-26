#include "Transformations.h"

#include <DirectXMath.h>

Transformations::Transformations() :
	m_position(0.f, 0.f, 0.f),
	m_rotation(0.f, 0.f, 0.f),
	m_scale(1.f, 1.f, 1.f)
{
}


Transformations::~Transformations()
{
}

DirectX::XMMATRIX Transformations::GetModel()
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_position));
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_rotation));
	DirectX::XMMATRIX model = scale * rotation * translation;

	DirectX::XMStoreFloat4x4(&m_model, model);

	return DirectX::XMLoadFloat4x4(&m_model);
}

void Transformations::Move(const DirectX::XMFLOAT3 &direction)
{
	m_position.x += direction.x;
	m_position.y += direction.y;
	m_position.z += direction.z;
}

void Transformations::Rotate(const DirectX::XMFLOAT3 &rotation)
{
	m_rotation.x = rotation.x;
	m_rotation.y = rotation.y;
	m_rotation.z = rotation.z;
}

void Transformations::Scale(const DirectX::XMFLOAT3 &scale)
{
	m_scale.x = scale.x;
	m_scale.y = scale.y;
	m_scale.z = scale.z;
}
