#pragma once
#include <memory>
#include <string>
#include <filesystem>

class FileManager;
class TextureManager;
class ShaderManager;

class ResourceManager {
public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void OnInit();

    virtual TextureManager* GetTextureManager() const;
    virtual FileManager* GetFileManager() const;
    virtual ShaderManager* GetShaderManager() const;

    virtual const std::filesystem::path& GetRootDir() const;

protected:
    std::unique_ptr<FileManager> m_fileMgr;
    std::unique_ptr<TextureManager> m_textureMgr;
    std::unique_ptr<ShaderManager> m_shaderMgr;

    std::filesystem::path m_root_dir;
};