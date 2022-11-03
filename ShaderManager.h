#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>                // COM helpers.
#include <dxc/dxcapi.h>         // Be sure to link with dxcompiler.lib.
#include <dxc/d3d12shader.h>    // Shader reflection.
#include <string>
#include <filesystem>
#include <vector>
#include "simple_object_pool.h"

using  Microsoft::WRL::ComPtr;

class ShaderManager {
public:
    enum ShaderType { st_vertex, st_pixel, st_compute};
    struct ShaderBlob {
        std::vector<uint8_t> data;
        std::wstring name;
    };
public:
    ShaderManager();

    ShaderManager::ShaderBlob* GetShaderBLOB(const std::wstring &name) {
        auto it = std::find_if(m_loaded_shaders.begin(), m_loaded_shaders.end(), [&name](ShaderBlob &blob){ return name == blob.name; });
        if (it != m_loaded_shaders.end())
            return &*it;
        else
            return nullptr;
    }
    ShaderManager::ShaderBlob* Load(const std::wstring &name, const std::wstring &entry_point, ShaderType target);

    const std::filesystem::path& GetShaderSourceDir() const;
    const std::filesystem::path& GetShaderBinaryDir() const;
private:
    pro_game_containers::simple_object_pool<ShaderBlob, 256> m_loaded_shaders;
    ComPtr<IDxcUtils> m_utils;
    ComPtr<IDxcCompiler3> m_compiler;
    ComPtr<IDxcIncludeHandler> m_includeHandler;

    std::filesystem::path m_shader_source_dir;
    std::filesystem::path m_shader_bin_dir;
};