#pragma once

#include "DirectXTex.h"

struct TextureData : protected pro_game_containers::object {
    DirectX::TexMetadata meta_data;
    DirectX::ScratchImage scratch_image;
    std::wstring name;
};
