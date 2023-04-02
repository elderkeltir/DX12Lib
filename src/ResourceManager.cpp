#include "ResourceManager.h"
#include "FileManager.h"
#include "GpuDataManager.h"
#include "MaterialManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager() = default;

void ResourceManager::OnInit(const std::filesystem::path& root_dir){
    m_root_dir = root_dir;

    m_fileMgr = std::make_shared<FileManager>();
    m_gpu_data_mgr = std::make_shared<GpuDataManager>();
    m_material_mgr = std::make_shared<MaterialManager>();
}

const std::filesystem::path& ResourceManager::GetRootDir() const{
    return m_root_dir;
}