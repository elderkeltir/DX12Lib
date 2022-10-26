#include "ResourceManager.h"

ResourceManager::ResourceManager() : 
    m_textureMgr(std::make_unique<TextureManager>()),
    m_fileMgr(std::make_unique<FileManager>()),
    m_shaderMgr(std::make_unique<ShaderManager>())
{

}

TextureManager* ResourceManager::GetTextureManager() const{
    return m_textureMgr.get();
}

FileManager* ResourceManager::GetFileManager() const {
    return m_fileMgr.get();
}

ShaderManager* ResourceManager::GetShaderManager() const {
    return m_shaderMgr.get();
}