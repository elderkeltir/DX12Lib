#pragma once

#include <string>
#include <memory>

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

    bool ReadModelFromFBX(const char * inFilePath, uint32_t id, RenderModel* outMesh, uint32_t *outMeshNum);
private:
    std::unique_ptr<Assimp::Importer> m_modelImporter;
};