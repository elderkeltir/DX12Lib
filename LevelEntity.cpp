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

    const char * model_name_8 = d["model"].GetString();
    const std::wstring model_name(&model_name_8[0], &model_name_8[strlen(model_name_8)]);
    if (std::shared_ptr<FileManager> fileMgr = gD3DApp->GetFileManager().lock()){
        m_model = fileMgr->LoadModel(model_name);
        m_model->SetName(model_name);
    }

    const Value &shaders = d["technique"];
    m_tech_id = shaders.GetInt();
}

void LevelEntity::Update(float dt){
    float angle = static_cast<float>(gD3DApp->TotalTime().count() * 90.0);
    const DirectX::XMVECTOR rot_axis  = DirectX::XMVectorSet(0, 1, 1, 0);
    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationAxis(rot_axis, DirectX::XMConvertToRadians(angle));
    DirectX::XMMATRIX pos = DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    DirectX::XMMATRIX xform = DirectX::XMMatrixMultiply(scale, rot);
    xform = DirectX::XMMatrixMultiply(xform, pos);

    DirectX::XMStoreFloat4x4(&m_xform, xform);
}

void LevelEntity::Render(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    m_model->Render(commandList, m_xform);
}

void LevelEntity::LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList) {
    m_model->LoadDataToGpu(commandList);
}