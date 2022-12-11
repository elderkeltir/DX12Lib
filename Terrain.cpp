#include "Terrain.h"
#include "FileManager.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;

void Terrain::Load(const std::wstring &hm_name)
{
	if (std::shared_ptr<FileManager> fileMgr = gD3DApp->GetFileManager().lock()) {
		RenderObject* &obj = (RenderObject*&)m_model;
		fileMgr->CreateModel(hm_name, FileManager::Geom_type::gt_triangle, obj);
	}

	m_model->SetInstancesNum(64*64*2);
	m_model->SetTechniqueId(7);
}

void Terrain::Render(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
	DirectX::XMFLOAT4X4 terrain_mx(
		1.f, .0f, .0f, .0f,
		.0f, 1.f, .0f, .0f,
		.0f, .0f, 1.f, .0f,
		.0f, .0f, .0f, 1.f
	);

	m_model->LoadDataToGpu(command_list);
	m_model->Render(command_list, terrain_mx);
}
