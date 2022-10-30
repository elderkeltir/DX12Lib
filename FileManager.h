#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <assimp/matrix4x4.h>
#include "simple_object_pool.h"

#include "RenderModel.h"

namespace Assimp
{
	class Importer;
    
}
struct aiScene;
struct aiNode;


// class RenderModel;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    RenderModel* LoadModel(const std::wstring &name);
    const std::filesystem::path& GetModelDir() const;
private:
    static constexpr uint32_t models_capacity = 256;

    void SetupModelRoot(const aiScene* scene, RenderModel* curr_model);
    void TraverseMeshes(const aiScene* scene, aiNode* rootNode, const aiMatrix4x4 &parent_trans, RenderModel* parent_model);
    RenderModel* AllocModel() { assert(m_model_count < models_capacity); m_load_meshes[m_model_count].SetId(m_model_count); return &(m_load_meshes[m_model_count++]); }
    RenderModel* LoadModelInternal(const std::wstring &name);
    void ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderModel* outModel);
    void InitializeModel(const aiScene* scene, const aiNode* rootNode, uint32_t meshesIdx, const aiMatrix4x4 &model_xform, RenderModel* outModel) const;

    std::unique_ptr<Assimp::Importer> m_modelImporter;
    pro_game_containers::simple_object_pool<RenderModel, models_capacity> m_load_meshes;
    std::filesystem::path m_model_dir;

    uint32_t m_model_count;
};