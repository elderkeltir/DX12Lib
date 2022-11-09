#pragma once
#include <memory>
#include <string>
#include <filesystem>

class FileManager;
class ShaderManager;
class GpuDataManager;

class ResourceManager {
public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void OnInit();

    virtual std::weak_ptr<FileManager> GetFileManager() const { return m_fileMgr; }
    virtual std::weak_ptr<ShaderManager> GetShaderManager() const { return m_shaderMgr; }
    virtual std::weak_ptr<GpuDataManager> GetGpuDatamanager() const { return m_gpu_data_mgr; }
    virtual const std::filesystem::path& GetRootDir() const;

protected:
    std::shared_ptr<FileManager> m_fileMgr;
    std::shared_ptr<ShaderManager> m_shaderMgr;
    std::shared_ptr<GpuDataManager> m_gpu_data_mgr;
    std::filesystem::path m_root_dir;
};