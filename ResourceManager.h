#pragma once
#include <memory>
#include <string>
#include <filesystem>

class FileManager;
class ShaderManager;

class ResourceManager {
public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void OnInit();

    virtual std::weak_ptr<FileManager> GetFileManager() const { return m_fileMgr; }
    virtual std::weak_ptr<ShaderManager> GetShaderManager() const { return m_shaderMgr; }

    virtual const std::filesystem::path& GetRootDir() const;

protected:
    std::shared_ptr<FileManager> m_fileMgr;
    std::shared_ptr<ShaderManager> m_shaderMgr;

    std::filesystem::path m_root_dir;
};