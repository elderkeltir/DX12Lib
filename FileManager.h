#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <array>

#include "RenderMesh.h"

namespace Assimp
{
	class Importer;
}


// class RenderMesh;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    const RenderMesh* LoadModel(const std::wstring &name);
    const std::filesystem::path& GetModelDir() const;
private:
    const RenderMesh* LoadModelInternal(const std::wstring &name);
    bool ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderMesh* outMesh, uint32_t *outMeshNum);    std::unique_ptr<Assimp::Importer> m_modelImporter;
    std::array<RenderMesh, 256> m_load_meshes;
    std::filesystem::path m_model_dir;

    uint32_t m_model_count;
};