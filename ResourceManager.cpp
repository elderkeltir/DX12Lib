#include "ResourceManager.h"
#include "ShaderManager.h"
#include "FileManager.h"
#include "TextureManager.h"

ResourceManager::ResourceManager()
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

void ResourceManager::OnInit(){
    m_textureMgr = std::make_shared<TextureManager>();
    m_fileMgr = std::make_shared<FileManager>();
    m_shaderMgr = std::make_shared<ShaderManager>();
}

const std::filesystem::path& ResourceManager::GetRootDir() const{
    return m_root_dir;
}