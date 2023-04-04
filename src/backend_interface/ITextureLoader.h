#pragma once

#include <string>
#include <filesystem>

class ICommandList;
class IGpuResource;

class ITextureLoader {
public:
    struct TextureData {
        std::wstring name;
    };

    virtual void OnInit() = 0;
    virtual ITextureLoader::TextureData* LoadTextureOnCPU(const std::wstring& name) = 0;
    virtual void LoadTextureOnGPU(ICommandList* command_list, IGpuResource* res, TextureData* tex_data) = 0;
    virtual ~ITextureLoader() = default;
};

ITextureLoader* CreateTextureLoader(const std::filesystem::path& root_dir);