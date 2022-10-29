#pragma once

#include <wrl.h>                // COM helpers.
#include <dxc/dxcapi.h>         // Be sure to link with dxcompiler.lib.
#include <dxc/d3d12shader.h>    // Shader reflection.
#include <string>
#include <filesystem>
#include <map>

using  Microsoft::WRL::ComPtr;

class ShaderManager {
public:
    enum ShaderType { st_vertex, st_pixel, st_compute};
public:
    ShaderManager();

    IDxcBlob* GetShaderBLOB(const std::wstring &name);
    void Load(const std::wstring &name, const std::wstring &entry_point, ShaderType target);

    const std::filesystem::path& GetShaderSourceDir() const;
    const std::filesystem::path& GetShaderBinaryDir() const;
private:
    std::map<std::wstring, ComPtr<IDxcBlob>> m_loaded_shaders;
    ComPtr<IDxcUtils> m_utils;
    ComPtr<IDxcCompiler3> m_compiler;
    ComPtr<IDxcIncludeHandler> m_includeHandler;

    std::filesystem::path m_shader_source_dir;
    std::filesystem::path m_shader_bin_dir;
};