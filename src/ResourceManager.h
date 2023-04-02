#pragma once
#include <memory>
#include <string>
#include <filesystem>

class FileManager;
class ShaderManager;
class GpuDataManager;
class MaterialManager;

class ResourceManager {
public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void OnInit(const std::filesystem::path& root_dir);

    virtual std::weak_ptr<FileManager> GetFileManager() const { return m_fileMgr; }
    virtual std::weak_ptr<GpuDataManager> GetGpuDataManager() const { return m_gpu_data_mgr; }
    virtual std::weak_ptr<MaterialManager> GetMaterialManager() const { return m_material_mgr; }
    virtual const std::filesystem::path& GetRootDir() const;

protected:
    std::shared_ptr<FileManager> m_fileMgr;
    std::shared_ptr<GpuDataManager> m_gpu_data_mgr;
    std::shared_ptr<MaterialManager> m_material_mgr;
    std::filesystem::path m_root_dir;
};