#include "FreeCamera.h"

void FreeCamera::Move(const DirectX::XMFLOAT3 &pos){
    m_pos = pos;
}

void FreeCamera::Rotate(const DirectX::XMFLOAT3 &dir){
    m_dir = dir;
}