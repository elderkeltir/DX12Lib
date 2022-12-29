#pragma once

#include <string>
#include <memory>
#include <array>
#include <filesystem>
#include <assimp/matrix4x4.h>
#include "simple_object_pool.h"
#include "RenderModel.h"
#include "free_allocator.h"

namespace Assimp
{
	class Importer;
    
}
struct aiScene;
struct aiNode;

class RenderQuad;

class FileManager
{
public:
    enum Geom_type { gt_sphere = 0, gt_quad, gt_triangle, gt_num };
    struct Geom {
        std::vector<DirectX::XMFLOAT3> vertices;
        std::vector<DirectX::XMFLOAT2> tex_coords;
        std::vector<uint16_t> indices;
        Geom_type type;
    };

    FileManager();
    ~FileManager();

    RenderModel* LoadModel(const std::wstring &name);
    void CreateModel(const std::wstring &tex_name, Geom_type type, RenderObject* &model);
    const std::filesystem::path& GetModelDir() const;
private:
    static constexpr uint32_t meshes_capacity = 256;
    static constexpr uint32_t textures_capacity = 128;

    void SetupModelRoot(const aiScene* scene, RenderModel* curr_model);
    void TraverseMeshes(const aiScene* scene, aiNode* rootNode, const aiMatrix4x4 &parent_trans, RenderModel* parent_model);
    bool AllocMesh(const std::wstring &name, RenderMesh* &mesh);
    RenderModel* LoadModelInternal(const std::wstring &name);
    void ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderModel* outModel);
    void InitializeModel(const aiScene* scene, const aiNode* rootNode, uint32_t meshesIdx, const aiMatrix4x4 &model_xform, RenderModel* outModel);
    TextureData*  LoadTexture(const std::wstring &name, uint32_t type);

    std::unique_ptr<Assimp::Importer> m_modelImporter;
    pro_game_containers::simple_object_pool<RenderModel, meshes_capacity * 2> m_load_models;
    pro_game_containers::simple_object_pool<RenderMesh, meshes_capacity> m_load_meshes;
    pro_game_containers::simple_object_pool<TextureData, textures_capacity> m_load_textures;
    std::array<Geom, gt_num> m_geoms;
    //std::array<std::wstring, gt_num> m_geom_name;

    std::filesystem::path m_model_dir;
    std::filesystem::path m_texture_dir;
};