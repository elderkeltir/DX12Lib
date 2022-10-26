#pragma once

#include <string>
#include <memory>
#include <filesystem>

namespace Assimp
{
	class Importer;
}


class RenderModel;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    bool ReadModelFromFBX(const char * name, uint32_t id, RenderModel* outMesh, uint32_t *outMeshNum);

    const std::filesystem::path& GetModelDir() const;
private:
    std::unique_ptr<Assimp::Importer> m_modelImporter;
    std::filesystem::path m_model_dir;
};