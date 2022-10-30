#include "FileManager.h"

#include <vector>
#include <assert.h>
#include <DirectXMath.h>
#include <cstdlib>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "RenderModel.h"
#include "DXAppImplementation.h"
#include "TextureManager.h"
#include "DXHelper.h"

extern DXAppImplementation *gD3DApp;

static const aiMatrix4x4 identity_mx(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
);

static constexpr uint32_t NO_MESH_IDX = (uint32_t)(-1);

RenderModel* FileManager::LoadModel(const std::wstring &name){
	const auto it = std::find_if(m_load_meshes.begin(), m_load_meshes.end(), [&name](RenderModel &mesh){ return (mesh.GetName() == name); });
	if (it != m_load_meshes.end()){
		return &(*it);
	}
	else {
		
		return LoadModelInternal(name);
	}
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
		RenderModel * curr_model = nullptr;
		for (uint32_t i = 0; i < rootNode->mNumMeshes; i++){
			uint32_t meshesIdx = rootNode->mMeshes[i];
			curr_model = AllocModel();
			const aiMatrix4x4 model_xform = (rootNode->mTransformation * parent_trans);
			InitializeModel(scene, rootNode, meshesIdx, model_xform, curr_model);
			parent_model->AddChild(curr_model);
		}
		parent_model = curr_model ? curr_model : parent_model;
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
	m_modelImporter(std::make_unique<Assimp::Importer>()),
	m_model_count(0)
{
	m_model_dir = gD3DApp->GetRootDir() / L"content" / L"models";
}


FileManager::~FileManager()
{
}

void FileManager::ReadModelFromFBX(const std::wstring &name, uint32_t id, RenderModel* outModel)
{
	std::filesystem::path file_path = m_model_dir / name;
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

void FileManager::InitializeModel(const aiScene* scene, const aiNode* rootNode, uint32_t meshesIdx, const aiMatrix4x4 &model_xform, RenderModel* outModel) const {
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
			uint32_t i = 0;
			aiMesh *mesh = scene->mMeshes[meshesIdx];
			uint32_t verticesNum = mesh->mNumVertices;
			uint32_t indicesNum = mesh->mNumFaces * 3;
			std::vector<DirectX::XMFLOAT3> vertices(verticesNum);
			std::vector<DirectX::XMFLOAT3> normals(verticesNum);
			std::vector<DirectX::XMFLOAT3> tangents(verticesNum);
			std::vector<DirectX::XMFLOAT3> bitangents(verticesNum);
			std::vector<uint16_t> indices(indicesNum);

			for (i = 0; i < verticesNum; i++)
			{
				vertices[i].x = mesh->mVertices[i].x;
				vertices[i].y = mesh->mVertices[i].y;
				vertices[i].z = mesh->mVertices[i].z;
			}
			outModel->SetVertices(std::move(vertices));

			for (uint32_t j = 0, i = 0; i < (uint32_t)mesh->mNumFaces; i++)
			{
				indices[j++] = mesh->mFaces[i].mIndices[0];
				indices[j++] = mesh->mFaces[i].mIndices[1];
				indices[j++] = mesh->mFaces[i].mIndices[2];
			}
			outModel->SetIndices(std::move(indices));

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

				outModel->SetNormals(std::move(normals));

				if (mesh->HasTangentsAndBitangents())
				{
					outModel->SetTangents(std::move(tangents), std::move(bitangents));
				}
			}

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

				for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType_DIFFUSE); k++)
				{
					assert(k == 0);
					aiString texturePath;

					if (material->GetTexture(aiTextureType_DIFFUSE, k, &texturePath) == aiReturn_SUCCESS)
					{
						// Check if it's an embedded or external  texture.
						if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							if (std::shared_ptr<TextureManager> textureMgr = gD3DApp->GetTextureManager().lock())
							{
								textureMgr->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
							}
							outModel->SetTexturePath(texturePath.C_Str(), RenderModel::TextureType::DiffuseTexture);
						}
					}
				}
				for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType_NORMALS); k++)
				{
					assert(k == 0);
					aiString texturePath;
					if (material->GetTexture(aiTextureType_NORMALS, k, &texturePath) == aiReturn_SUCCESS)
					{
						if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							if (std::shared_ptr<TextureManager> textureMgr = gD3DApp->GetTextureManager().lock())
							{
								textureMgr->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
							}
							outModel->SetTexturePath(texturePath.C_Str(), RenderModel::TextureType::NormalTexture);
							// outModel->SetFlag(Mesh::MeshFlags::UseNormalMap);
						}
					}
				}
				for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType_SPECULAR); k++)
				{
					assert(k == 0);
					aiString texturePath;
					if (material->GetTexture(aiTextureType_SPECULAR, k, &texturePath) == aiReturn_SUCCESS)
					{
						if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str()))
						{
							assert(false);
						}
						else
						{
							if (std::shared_ptr<TextureManager> textureMgr = gD3DApp->GetTextureManager().lock())
							{
								textureMgr->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
							}
							outModel->SetTexturePath(texturePath.C_Str(), RenderModel::TextureType::SpecularTexture);
							// outModel->SetFlag(Mesh::MeshFlags::UseSpecularMap);
						}
					}
				}

				// texture coords
				if (mesh->mTextureCoords)
				{
					std::vector<DirectX::XMFLOAT2> textCoords_(mesh->mNumVertices);

					for (uint32_t k = 0; k < mesh->mNumVertices; k++)
					{
						textCoords_[k].x = mesh->mTextureCoords[0][k].x;
						textCoords_[k].y = mesh->mTextureCoords[0][k].y;
					}

					outModel->SetTextureCoords(std::move(textCoords_));
				}
			}
		}

		outModel->Initialized();
	}
}

const std::filesystem::path& FileManager::GetModelDir() const{
	return m_model_dir;
}

RenderModel* FileManager::LoadModelInternal(const std::wstring &name){
	RenderModel* new_model = AllocModel();
	uint32_t cnt = 0;
	ReadModelFromFBX(name, 0, new_model);
	assert(new_model->IsInitialized());

	return new_model;
}