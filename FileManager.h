#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <array>

#include "RenderModel.h"

namespace Assimp
{
	class Importer;
}


// class RenderModel;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    RenderModel* LoadModel(const std::wstring &name);
    const std::filesystem::path& GetModelDir() const;
private:
    RenderModel* LoadModelInternal(const std::wstring &name);
    bool ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderModel* outMesh, uint32_t *outMeshNum);
    std::unique_ptr<Assimp::Importer> m_modelImporter;
    std::array<RenderModel, 256> m_load_meshes;
    std::filesystem::path m_model_dir;

    uint32_t m_model_count;
};