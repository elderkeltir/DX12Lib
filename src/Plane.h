#pragma once

#include <memory>
#include <string>
#include <DirectXMath.h>


class RenderModel;
class IGpuResource;
class ICommandList;

class Plane {
public:
	void Load(const std::wstring& hm_name, uint32_t dim, uint32_t tech_id, const DirectX::XMFLOAT4& pos);
	void Render(ICommandList* command_list);
	uint32_t GetTerrainDim() const { return m_plane_dim; }
	uint32_t GetTerrainTechId() const { return m_tech_id; }
private:
	DirectX::XMFLOAT4 m_pos;
	uint32_t m_plane_dim;
	uint32_t m_tech_id;
	RenderModel* m_model;
};