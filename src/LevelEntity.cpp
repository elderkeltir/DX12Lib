#include "LevelEntity.h"
#if defined(min)
#undef min
#endif
 
#if defined(max)
#undef max
#endif
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>

#include "DXHelper.h"
#include "DXAppImplementation.h"
#include "RenderModel.h"
#include "FileManager.h"
#include "Level.h"
#include "MaterialManager.h"
#include "CommandQueue.h"

extern DXAppImplementation *gD3DApp;
using rapidjson::Document;
using rapidjson::Value;

LevelEntity::LevelEntity(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &rot, const DirectX::XMFLOAT3 &scale) :
    m_model(nullptr),
    m_pos(pos),
    m_rot(rot),
    m_scale(scale)
{

}

void LevelEntity::Load(const std::wstring &name){
    // read file
    std::string content;
    {
        if (std::shared_ptr<Level> level =  gD3DApp->GetLevel().lock()){
            const std::filesystem::path fullPath = (level->GetEntitiesDir() / name);
            std::ifstream ifs(fullPath.wstring());
            content.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        }
    }

    // parse file
    Document d;
    d.Parse(content.c_str());

    const Value &shaders = d["technique"];
    m_tech_id = shaders.GetInt();

    const char * model_name_8 = d["model"].GetString();
    const std::wstring model_name(&model_name_8[0], &model_name_8[strlen(model_name_8)]);
    if (std::shared_ptr<FileManager> fileMgr = gD3DApp->GetFileManager().lock()){
        m_model = fileMgr->LoadModel(model_name);
        m_model->SetName(model_name);
        m_model->SetTechniqueId(m_tech_id);

        if (gD3DApp->TechHasColor(m_tech_id)){
            const Value &color_val = d["color"];
            const DirectX::XMFLOAT3 color(color_val[0].GetFloat(), color_val[1].GetFloat(), color_val[2].GetFloat());
            m_model->SetColor(color);

            // load material
			const Value& material = d["material"];
			const Value& metallic = material["metallic"];
			const Value& roughness = material["roughness"];
            const Value& reflectivity = material["reflectivity"];
			if (std::shared_ptr<MaterialManager> mat_mgr = gD3DApp->GetMaterialManager().lock()) {
				uint32_t mat_id = mat_mgr->CreateMaterial(metallic.GetFloat(), roughness.GetFloat(), reflectivity.GetFloat());
				m_model->SetMaterial(mat_id);
			}
        }
    }
}

void LevelEntity::Update(float dt){
    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(m_rot.x), DirectX::XMConvertToRadians(m_rot.y), DirectX::XMConvertToRadians(m_rot.z));
    DirectX::XMMATRIX pos = DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    DirectX::XMMATRIX xform = DirectX::XMMatrixMultiply(scale, rot);
    xform = DirectX::XMMatrixMultiply(xform, pos);

    DirectX::XMStoreFloat4x4(&m_xform, xform);
}

void LevelEntity::Render(CommandList& command_list){
    m_model->Render(command_list, m_xform);
}

void LevelEntity::LoadDataToGpu(CommandList& command_list) {
    m_model->LoadDataToGpu(command_list);
}