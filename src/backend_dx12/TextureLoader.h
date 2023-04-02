#pragma once

#include "ITextureLoader.h"
#include "DirectXTex.h"
#include "simple_object_pool.h"
#include <filesystem>

class TextureLoader : public ITextureLoader {
public:
    struct TextureDataDx : ITextureLoader::TextureData {
        DirectX::TexMetadata meta_data;
        DirectX::ScratchImage scratch_image;
    };
    TextureLoader(const std::filesystem::path& root_dir);
    void OnInit() override;
    ITextureLoader::TextureData* LoadTextureOnCPU(const std::wstring& name) override;
    void LoadTextureOnGPU(ICommandList* command_list, IGpuResource* res, ITextureLoader::TextureData* tex_data) override;

private:
    static constexpr uint32_t textures_capacity = 128;
    pro_game_containers::simple_object_pool<TextureDataDx, textures_capacity> m_load_textures;
    std::filesystem::path m_texture_dir;
};