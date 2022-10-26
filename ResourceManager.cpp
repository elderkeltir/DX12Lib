#include "ResourceManager.h"
#include "ShaderManager.h"
#include "FileManager.h"
#include "TextureManager.h"

ResourceManager::ResourceManager() : 
    m_textureMgr(std::make_unique<TextureManager>()),
    m_fileMgr(std::make_unique<FileManager>()),
    m_shaderMgr(std::make_unique<ShaderManager>())
{
    // exe path
    wchar_t executablePath[MAX_PATH];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule)
    {
        GetModuleFileName(hModule, executablePath, sizeof(wchar_t) * MAX_PATH);
        std::filesystem::path p = std::filesystem::path(executablePath);
	    m_root_dir = p.parent_path().parent_path().parent_path();
    }
}

ResourceManager::~ResourceManager() = default;

TextureManager* ResourceManager::GetTextureManager() const{
    return m_textureMgr.get();
}

FileManager* ResourceManager::GetFileManager() const {
    return m_fileMgr.get();
}

ShaderManager* ResourceManager::GetShaderManager() const {
    return m_shaderMgr.get();
}

const std::filesystem::path& ResourceManager::GetRootDir() const{
    return m_root_dir;
}