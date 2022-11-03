#include "Level.h"

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

#include "FreeCamera.h"
#include "DXAppImplementation.h"
#include "FileManager.h"

extern DXAppImplementation *gD3DApp;
using rapidjson::Document;
using rapidjson::Value;

Level::Level()
{
    m_levels_dir = gD3DApp->GetRootDir() / L"content" / L"levels";
    m_entities_dir = gD3DApp->GetRootDir() / L"content" / L"entities";
}
Level::~Level() = default;

void Level::Load(const std::wstring &name){
    m_name = name;
    
    // read file
    std::string content;
    {
        const std::filesystem::path fullPath = (m_levels_dir / name);
        std::ifstream ifs(fullPath.wstring());
        content.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    }

    // parse file
    Document d;
    d.Parse(content.c_str());

    // camera
    {
        const Value &camera = d["camera"];
        const Value &camera_pos = camera["pos"];
        const Value &camera_dir = camera["dir"];
        const DirectX::XMFLOAT3 pos(camera_pos[0].GetFloat(), camera_pos[1].GetFloat(), camera_pos[2].GetFloat());
        const DirectX::XMFLOAT3 dir(camera_dir[0].GetFloat(), camera_dir[1].GetFloat(), camera_dir[2].GetFloat());
        
        const Value &camera_fov = camera["fov"];
        const Value &camera_near = camera["near"];
        const Value &camera_far = camera["far"];
        
        m_camera.swap(std::make_shared<FreeCamera>(camera_fov.GetFloat(), camera_near.GetFloat(), camera_far.GetFloat(), gD3DApp->GetAspectRatio()));
        m_camera->Move(pos);
        m_camera->Rotate(dir);
    }

    // entities
    const Value& entities = d["entities"];
    for (uint32_t i = 0; i < entities.Size(); i++){
        const Value& entity = entities[i];
        const char * model_name_8 = entity["model"].GetString();
        const std::wstring model_name(&model_name_8[0], &model_name_8[strlen(model_name_8)]);

        const Value& model_pos = entity["pos"];
        const Value& model_rot = entity["rot"];
        const Value& model_scale = entity["scale"];
        const DirectX::XMFLOAT3 pos(model_pos[0].GetFloat(), model_pos[1].GetFloat(), model_pos[2].GetFloat());
        const DirectX::XMFLOAT3 rot(model_rot[0].GetFloat(), model_rot[1].GetFloat(), model_rot[2].GetFloat());
        const DirectX::XMFLOAT3 scale(model_scale[0].GetFloat(), model_scale[1].GetFloat(), model_scale[2].GetFloat());

        LevelEntity lev_ent{ L"", pos, rot, scale };
        lev_ent.Load(model_name);
        uint32_t id = m_entites.push_back(lev_ent);
        m_entites[id].SetId(id);
    }

    // Lights
    const Value& lights = d["lights"];
    for (uint32_t i = 0; i < lights.Size(); i++){
        const Value& light = lights[i];
        const LevelLight::LightType ltype = static_cast<LevelLight::LightType>(light["type"].GetInt());

        if (ltype == LevelLight::LightType::lt_direct){
            // const Value& light_pos = light["pos"]; NOT FOR DIRECTION LIGHT
            const Value& light_dir = light["dir"];
            const Value& light_power = light["power"];
            // const DirectX::XMFLOAT3 pos(light_pos[0].GetFloat(), light_pos[1].GetFloat(), light_pos[2].GetFloat());
            const DirectX::XMFLOAT3 dir(light_dir[0].GetFloat(), light_dir[1].GetFloat(), light_dir[2].GetFloat());
            const DirectX::XMFLOAT3 power(light_power[0].GetFloat(), light_power[1].GetFloat(), light_power[2].GetFloat());

            const LevelLight level_light{ DirectX::XMFLOAT3(), dir, power, 0, ltype };
            uint32_t id = m_lights.push_back(level_light);
            m_lights[id].id = id;
        }
    }
}

void Level::Update(float dt){
    // update camera
    m_camera->Update(dt);

    //update entites
    for (auto &entity : m_entites){
        entity.Update(dt);
    }
}

void Level::Render(){

}

const std::filesystem::path& Level::GetLevelsDir() const{
    return m_levels_dir;
}

const std::filesystem::path& Level::GetEntitiesDir() const{
    return m_entities_dir;
}