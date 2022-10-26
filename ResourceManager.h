#pragma once
#include <memory>

#include "FileManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"

class ResourceManager {
public:
    ResourceManager();

    virtual TextureManager* GetTextureManager() const;
    virtual FileManager* GetFileManager() const;
    virtual ShaderManager* GetShaderManager() const;
protected:
    std::unique_ptr<FileManager> m_fileMgr;
    std::unique_ptr<TextureManager> m_textureMgr;
    std::unique_ptr<ShaderManager> m_shaderMgr;
};