#pragma once

#include "DirectXTex.h"

struct TextureData {
    DirectX::TexMetadata meta_data;
    DirectX::ScratchImage scratch_image;
    std::wstring name;
};
