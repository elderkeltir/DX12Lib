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

extern DXAppImplementation *gD3DApp;

struct MiniMesh
{
	aiMatrix4x4 transformations;
	unsigned int numMeshes;
	unsigned int* meshesIdx;
};

void TraverseMeshes(aiNode* rootNode, const aiMatrix4x4 &trans, std::vector<MiniMesh> &meshes)
{
	aiMatrix4x4 currTrans = rootNode->mTransformation * trans;
	if (rootNode->mNumMeshes)
	{
		MiniMesh mesh;
		mesh.meshesIdx = rootNode->mMeshes;
		mesh.numMeshes = rootNode->mNumMeshes;
		mesh.transformations = currTrans;

		meshes.push_back(mesh);
	}

	for (uint32_t i = 0; i < rootNode->mNumChildren; i++)
	{
		TraverseMeshes(rootNode->mChildren[i], currTrans, meshes);
	}
}

FileManager::FileManager() : 
	m_modelImporter(std::make_unique<Assimp::Importer>())
{
	m_model_dir = gD3DApp->GetRootDir() / L"content" / L"models";
}


FileManager::~FileManager()
{
}

bool FileManager::ReadModelFromFBX(const char * name, uint32_t id, RenderModel* outModel, uint32_t *outModelNum)
{
	std::wstring name_long(&name[0], &name[strlen(name)]);
	std::string file_path = (m_model_dir / name_long).u8string();
	// fetch data
	const aiScene* scene = m_modelImporter->ReadFile(file_path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType);

	if (scene)
	{
		// nodes
		aiNode* rootNode = scene->mRootNode;

		std::vector<MiniMesh> tempMeshes;
		aiMatrix4x4 currTrans;

		TraverseMeshes(rootNode, currTrans, tempMeshes);
		for (const auto &miniM : tempMeshes)
		{
			if (id == *miniM.meshesIdx)
			{
				aiVector3D rot;
				aiVector3D scale;
				aiVector3D trans;

				miniM.transformations.Decompose(scale, rot, trans);

				DirectX::XMFLOAT3 DXrot(rot.x, rot.y, rot.z);
				DirectX::XMFLOAT3 DXscale(scale.x, scale.y, scale.z);
				DirectX::XMFLOAT3 DXtrans(trans.x, trans.y, trans.z);

				outModel->Move(DXtrans);
				outModel->Scale(DXscale);
				outModel->Rotate(DXrot);
			}
		}

		// meshes
		uint32_t i = 0;
		uint32_t j = 0;
		*outModelNum = scene->mNumMeshes;
		aiMesh* mesh = scene->mMeshes[id];
		uint32_t verticesNum = mesh->mNumVertices;
		uint32_t indicesNum = mesh->mNumFaces * 3;
		std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<DirectX::XMFLOAT3[]> normals = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<DirectX::XMFLOAT3[]> tangents = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<DirectX::XMFLOAT3[]> bitangents = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<uint32_t[]> indices = std::make_unique<uint32_t[]>(indicesNum);
		
		for (; i < verticesNum; i++)
		{
			vertices[i].x = mesh->mVertices[i].x;
			vertices[i].y = mesh->mVertices[i].y;
			vertices[i].z = mesh->mVertices[i].z;
		}
		outModel->SetVertices(std::move(vertices), verticesNum);

		for (i = 0; i < mesh->mNumFaces; i++)
		{
			indices[j++] = mesh->mFaces[i].mIndices[0];
			indices[j++] = mesh->mFaces[i].mIndices[1];
			indices[j++] = mesh->mFaces[i].mIndices[2];
		}
		outModel->SetIndices(std::move(indices), indicesNum);

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
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
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

				if (material->GetTexture(aiTextureType_DIFFUSE, k, &texturePath) ==	aiReturn_SUCCESS)
				{
					// Check if it's an embedded or external  texture.
					if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str()))
					{
						assert(false);
					}
					else
					{
                        gD3DApp->GetTextureManager()->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
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
						gD3DApp->GetTextureManager()->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
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
						gD3DApp->GetTextureManager()->AddTexture(std::wstring(&texturePath.C_Str()[0], &texturePath.C_Str()[strlen(texturePath.C_Str())]));
						outModel->SetTexturePath(texturePath.C_Str(), RenderModel::TextureType::SpecularTexture);
						// outModel->SetFlag(Mesh::MeshFlags::UseSpecularMap);
					}
				}
			}

			// texture coords
			if (mesh->mTextureCoords)
			{
				std::unique_ptr<DirectX::XMFLOAT2[]> textCoords_ = std::make_unique<DirectX::XMFLOAT2[]>(mesh->mNumVertices);

				for (uint32_t k = 0; k < mesh->mNumVertices; k++)
				{
					textCoords_[k].x = mesh->mTextureCoords[0][k].x;
					textCoords_[k].y = mesh->mTextureCoords[0][k].y;
				}

				outModel->SetTextureCoords(std::move(textCoords_));
			}
		}

		return true;
	}

	return false;
}

const std::filesystem::path& FileManager::GetModelDir() const{
	return m_model_dir;
}