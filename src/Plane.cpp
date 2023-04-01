#include "Plane.h"
#include "FileManager.h"
#include "Frontend.h"
#include "ICommandList.h"

extern Frontend* gFrontend;

void Plane::Load(const std::wstring &hm_name, uint32_t dim, uint32_t tech_id, const DirectX::XMFLOAT4& pos)
{
	m_pos = pos;
	m_plane_dim = dim;
	m_tech_id = tech_id;
	if (std::shared_ptr<FileManager> fileMgr = gFrontend->GetFileManager().lock()) {
		RenderObject* &obj = (RenderObject*&)m_model;
		fileMgr->CreateModel(hm_name, FileManager::Geom_type::gt_triangle, obj);
	}

	m_model->SetInstancesNum(m_plane_dim * m_plane_dim);
	m_model->SetTechniqueId(GetTerrainTechId());
}

void Plane::Render(ICommandList* command_list)
{
	DirectX::XMFLOAT4X4 terrain_mx(
		1.f, .0f, .0f, .0f,
		.0f, 1.f, .0f, .0f,
		.0f, .0f, 1.f, .0f,
		m_pos.x, m_pos.y, m_pos.z, 1.f
	);

	m_model->LoadDataToGpu(command_list);
	m_model->Render(command_list, terrain_mx);
}
