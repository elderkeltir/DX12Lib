#pragma once

#include "DirectXTex.h"
#include "simple_object_pool.h"

struct TextureData  {
    DirectX::TexMetadata meta_data;
    DirectX::ScratchImage scratch_image;
    std::wstring name;
};
