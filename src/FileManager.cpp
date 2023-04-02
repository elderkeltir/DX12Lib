#include "FileManager.h"

#include <vector>
#include <assert.h>
#include <DirectXMath.h>
#include <cstdlib>

#ifdef _DEBUG
#include <sstream>
#endif // _DEBUG

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "RenderModel.h"
#include "RenderQuad.h"
#include "Frontend.h"
#include "GeomUtils.h"

extern Frontend* gFrontend;

static const aiMatrix4x4 identity_mx(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
);

#define ThrowIfFailed(exp) exp // TODO: hack

static constexpr uint32_t NO_MESH_IDX = (uint32_t)(-1);

bool FileManager::AllocMesh(const std::wstring &name, RenderMesh* &mesh){
	const auto it = std::find_if(m_load_meshes.begin(), m_load_meshes.end(), [&name](RenderMesh &mesh){ return (mesh.GetName() == name); });
	if (it != m_load_meshes.end()){
		mesh = &(*it);

		return false;
	}
	else {
		const uint32_t idx = m_load_meshes.push_back(); 
		m_load_meshes[idx].SetId(idx); 
		mesh = &(m_load_meshes[idx]);
		mesh->SetName(name);

		return true;
	}
}

RenderModel* FileManager::LoadModel(const std::wstring &name){
	return LoadModelInternal(name);
}

void FileManager::SetupModelRoot(const aiScene* scene, RenderModel* curr_model){
	aiNode* rootNode = scene->mRootNode;
	aiMatrix4x4 model_xform(rootNode->mTransformation);
	assert(rootNode->mNumMeshes < 2);
	uint32_t mesh_idx = rootNode->mNumMeshes ? rootNode->mMeshes[0] : NO_MESH_IDX;
	InitializeModel(scene, rootNode, mesh_idx, model_xform, curr_model);
}

void FileManager::TraverseMeshes(const aiScene* scene, aiNode* rootNode, const aiMatrix4x4 &parent_trans, RenderModel* parent_model)
{
	aiMatrix4x4 trans_for_children(identity_mx);
	if (rootNode->mNumMeshes)
	{
		RenderModel* curr_model = nullptr;
		for (uint32_t i = 0; i < rootNode->mNumMeshes; i++){
			uint32_t meshesIdx = rootNode->mMeshes[i];

			uint32_t id = m_load_models.push_back();
			curr_model = &m_load_models[id];

			const std::wstring curr_name(&rootNode->mName.C_Str()[0], &rootNode->mName.C_Str()[strlen(rootNode->mName.C_Str())]);
			curr_model->SetName(curr_name);
			const aiMatrix4x4 model_xform = (rootNode->mTransformation * parent_trans);
			InitializeModel(scene, rootNode, meshesIdx, model_xform, curr_model);
			parent_model->AddChild(curr_model);
		}
		parent_model = curr_model;
		assert(parent_model);
	}
	else {
		// stack xform's of nodes without meshes only.
		trans_for_children = rootNode->mTransformation * parent_trans;
	}

	// travers children
	
	for (uint32_t i = 0; i < rootNode->mNumChildren; i++)
	{
		TraverseMeshes(scene, rootNode->mChildren[i], trans_for_children, parent_model);
	}
}

FileManager::FileManager() : 
	m_modelImporter(std::make_unique<Assimp::Importer>())
{
	m_model_dir = gFrontend->GetRootDir() / L"content" / L"models";
	m_texture_loader.reset(CreateTextureLoader(gFrontend->GetRootDir()));
	m_texture_loader->OnInit();

	CreateSphere(m_geoms[gt_sphere].vertices, m_geoms[gt_sphere].indices);
	m_geoms[gt_sphere].type = gt_sphere;
	CreateTriangle(m_geoms[gt_triangle].indices);
	m_geoms[gt_triangle].type = gt_triangle;
}


FileManager::~FileManager()
{
}

void FileManager::ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderModel* outModel)
{
	std::filesystem::path file_path = m_model_dir / name;
	assert(std::filesystem::exists(file_path));
	// fetch data
	const aiScene* scene = m_modelImporter->ReadFile(file_path.u8string(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_ConvertToLeftHanded );
	if (scene) {
		aiNode* rootNode = scene->mRootNode;
		SetupModelRoot(scene, outModel);
		aiMatrix4x4 currTrans(identity_mx);
		TraverseMeshes(scene, rootNode, currTrans, outModel);
	}
}

void FileManager::InitializeModel(const aiScene* scene, const aiNode* rootNode, uint32_t meshesIdx, const aiMatrix4x4 &model_xform, RenderModel* outModel) {
	if (scene)
	{
		// xform
		aiVector3D rot;
		aiVector3D scale;
		aiVector3D trans;
		model_xform.Decompose(scale, rot, trans);

		DirectX::XMFLOAT3 DXrot(rot.x, rot.y, rot.z);
		DirectX::XMFLOAT3 DXscale(scale.x, scale.y, scale.z);
		DirectX::XMFLOAT3 DXtrans(trans.x, trans.y, trans.z);

		outModel->Move(DXtrans);
		outModel->Scale(DXscale);
		outModel->Rotate(DXrot);

		// mesh
		if (meshesIdx != NO_MESH_IDX)
		{
			aiMesh *mesh = scene->mMeshes[meshesIdx];
			uint32_t i = 0;
			uint32_t verticesNum = mesh->mNumVertices;
			uint32_t indicesNum = mesh->mNumFaces * 3;

			std::vector<DirectX::XMFLOAT3> normals(verticesNum);
			std::vector<DirectX::XMFLOAT3> tangents(verticesNum);
			std::vector<DirectX::XMFLOAT3> bitangents(verticesNum);

			RenderMesh* r_mesh = nullptr;
			if (AllocMesh(outModel->GetName(), r_mesh)) {
				std::vector<DirectX::XMFLOAT3> vertices(verticesNum);

				std::vector<uint16_t> indices(indicesNum);

				for (i = 0; i < verticesNum; i++)
				{
					vertices[i].x = mesh->mVertices[i].x;
					vertices[i].y = mesh->mVertices[i].y;
					vertices[i].z = mesh->mVertices[i].z;
				}
				r_mesh->SetVertices(std::move(vertices));

				for (uint32_t j = 0, i = 0; i < (uint32_t)mesh->mNumFaces; i++)
				{
					indices[j++] = mesh->mFaces[i].mIndices[0];
					indices[j++] = mesh->mFaces[i].mIndices[1];
					indices[j++] = mesh->mFaces[i].mIndices[2];
				}
				r_mesh->SetIndices(std::move(indices));

				// texture coords
				if (mesh->mTextureCoords)
				{
					std::vector<DirectX::XMFLOAT2> textCoords_(mesh->mNumVertices);

					for (uint32_t k = 0; k < mesh->mNumVertices; k++)
					{
						textCoords_[k].x = mesh->mTextureCoords[0][k].x;
						textCoords_[k].y = mesh->mTextureCoords[0][k].y;
					}

					r_mesh->SetTextureCoords(std::move(textCoords_));
				}

				if (mesh->HasNormals())
				{
					for (i = 0; i < verticesNum; i++)
					{
						normals[i].x = mesh->mNormals[i].x;
						normals[i].y = mesh->mNormals[i].y;
						normals[i].z = mesh->mNormals[i].z;

						if (mesh->HasTangentsAndBitangents())
						{
							tangents[i].x = mesh->mTangents[i].x;
							tangents[i].y = mesh->mTangents[i].y;
							tangents[i].z = mesh->mTangents[i].z;

							bitangents[i].x = mesh->mBitangents[i].x;
							bitangents[i].y = mesh->mBitangents[i].y;
							bitangents[i].z = mesh->mBitangents[i].z;
						}
					}

					r_mesh->SetNormals(std::move(normals));

					if (mesh->HasTangentsAndBitangents())
					{
						r_mesh->SetTangents(std::move(tangents), std::move(bitangents));
					}
				}
			}

			outModel->SetMesh(r_mesh);

			// materials, textures
			if (scene->mNumMaterials)
			{
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				aiColor4D specularColor;
				aiColor4D diffuseColor;
				aiColor4D ambientColor;
				float shininess;

				aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor);
				aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
				aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
				aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
#ifdef _DEBUG
				for (unsigned int g = 0; g < 22; g++){
					for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType(g)); k++) {
						aiString texturePath;
						material->GetTexture(aiTextureType(g), k, &texturePath);
						std::ostringstream ss;
						ss << rootNode->mName.C_Str() << " << TextureType=" << g << ", count=" << material->GetTextureCount(aiTextureType(g)) << ", name:" << texturePath.C_Str();
						std::string str = ss.str();
					}
				}
#endif // _DEBUG
				aiTextureType diffuse_tex = material->GetTextureCount(aiTextureType_DIFFUSE) ? aiTextureType_DIFFUSE : aiTextureType_BASE_COLOR;
				for (unsigned int k = 0; k < material->GetTextureCount(diffuse_tex); k++)
				{
					assert(k == 0);
					aiString texturePath;

					if (material->GetTexture(diffuse_tex, k, &texturePath) == aiReturn_SUCCESS)
					{
						// Check if it's an embedded or external  texture.
						if (auto texture_embeded = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							const std::wstring texture_path(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]);
							ITextureLoader::TextureData* texture_data = m_texture_loader->LoadTextureOnCPU(texture_path);
							assert(texture_data);
							outModel->SetTexture(texture_data, RenderModel::TextureType::DiffuseTexture);
						}
					}
				}
				aiTextureType normal_tex = material->GetTextureCount(aiTextureType_NORMALS) ? aiTextureType_NORMALS : aiTextureType_NORMAL_CAMERA;
				for (unsigned int k = 0; k < material->GetTextureCount(normal_tex); k++)
				{
					assert(k == 0);
					aiString texturePath;
					if (material->GetTexture(normal_tex, k, &texturePath) == aiReturn_SUCCESS)
					{
						if (auto texture_embeded = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							const std::wstring texture_path(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]);
							ITextureLoader::TextureData* texture_data = m_texture_loader->LoadTextureOnCPU(texture_path);
							assert(texture_data);
							outModel->SetTexture(texture_data, RenderModel::TextureType::NormalTexture);
						}
					}
				}
				for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType_METALNESS); k++)
				{
					assert(k == 0);
					aiString texturePath;
					if (material->GetTexture(aiTextureType_METALNESS, k, &texturePath) == aiReturn_SUCCESS)
					{
						if (auto texture_embeded = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							const std::wstring texture_path(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]);
							ITextureLoader::TextureData* texture_data = m_texture_loader->LoadTextureOnCPU(texture_path);
							assert(texture_data);
							outModel->SetTexture(texture_data, RenderModel::TextureType::MetallicTexture);
						}
					}
				}
				aiTextureType roughness_tex = material->GetTextureCount(aiTextureType_SHININESS) ? aiTextureType_SHININESS : aiTextureType_DIFFUSE_ROUGHNESS;
				for (unsigned int k = 0; k < material->GetTextureCount(roughness_tex); k++)
				{
					assert(k == 0);
					aiString texturePath;
					if (material->GetTexture(roughness_tex, k, &texturePath) == aiReturn_SUCCESS)
					{
						if (auto texture_embeded = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							const std::wstring texture_path(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]);
							ITextureLoader::TextureData* texture_data = m_texture_loader->LoadTextureOnCPU(texture_path);
							assert(texture_data);
							outModel->SetTexture(texture_data, RenderModel::TextureType::RoughTexture);
						}
					}
				}
			}
		}
		outModel->Initialized();
	}
}

const std::filesystem::path& FileManager::GetModelDir() const{
	return m_model_dir;
}

void FileManager::LoadTextureOnGPU(ICommandList* command_list, IGpuResource* res, ITextureLoader::TextureData* tex_data)
{
	m_texture_loader->LoadTextureOnGPU(command_list, res, tex_data);
}

RenderModel* FileManager::LoadModelInternal(const std::wstring &name){
	uint32_t idx = m_load_models.push_back();
	RenderModel *new_model = &m_load_models[idx];
	uint32_t cnt = 0;
	ReadModelFromFBX(name, 0, new_model);
	assert(new_model->IsInitialized());

	return new_model;
}

void FileManager::CreateModel(const std::wstring &tex_name, Geom_type type, RenderObject*& model) {
	if (!model){
		const uint32_t id = m_load_models.push_back();
		model = &m_load_models[id];
		model->SetId(id);
	}

	if (type != gt_quad) {
		const uint32_t idx = m_load_meshes.push_back();
		RenderMesh* r_mesh = &m_load_meshes[idx];
		r_mesh->SetId(idx);
		r_mesh->SetVertices(m_geoms[type].vertices);
		r_mesh->SetIndices(m_geoms[type].indices);
		r_mesh->SetTextureCoords(m_geoms[type].tex_coords);
		model->SetMesh(r_mesh);
	}

	if (!tex_name.empty()){
		ITextureLoader::TextureData* texture_data = m_texture_loader->LoadTextureOnCPU(tex_name);
		assert(texture_data);
		model->SetTexture(texture_data, RenderModel::TextureType::DiffuseTexture);
	}
	model->Initialized();
}