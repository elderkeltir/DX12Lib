#include "Level.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>

#include "FreeCamera.h"
#include "DXAppImplementation.h"
#include "RenderMesh.h"
#include "FileManager.h"
#include "ShaderManager.h"

extern DXAppImplementation *gD3DApp;
using namespace rapidjson;

void Level::LevelEntity::Load(const std::wstring &name){
    // read file
    std::string content;
    {
        const std::filesystem::path fullPath = (gD3DApp->GetLevel()->GetEntitiesDir() / name);
        std::ifstream ifs(fullPath.wstring());
        content.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    }

    // parse file
    Document d;
    d.Parse(content.c_str());

    const char * model_name_8 = d["model"].GetString();
    const std::wstring model_name(&model_name_8[0], &model_name_8[strlen(model_name_8)]);

    mesh = gD3DApp->GetFileManager()->LoadModel(model_name);

    const Value &textures = d["textures"];
    const char * difuse_name_8 = textures["difuse"].GetString();
    const std::wstring difuse_name(&difuse_name_8[0], &difuse_name_8[strlen(difuse_name_8)]);
    const char * normal_name_8 = textures["normal"].GetString();
    const std::wstring normal_name(&normal_name_8[0], &normal_name_8[strlen(normal_name_8)]);
    const char * specular_name_8 = textures["specular"].GetString();
    const std::wstring specular_name(&specular_name_8[0], &specular_name_8[strlen(specular_name_8)]);

    const Value &shaders = d["shaders"];
    const char * vertex_name_8 = shaders["vertex"].GetString();
    const std::wstring vertex_name(&vertex_name_8[0], &vertex_name_8[strlen(vertex_name_8)]);
    const char * pixel_name_8 = shaders["pixel"].GetString();
    const std::wstring pixel_name(&pixel_name_8[0], &pixel_name_8[strlen(pixel_name_8)]);

    gD3DApp->GetShaderManager()->Load(vertex_name, L"main", ShaderManager::ShaderType::st_vertex);
    gD3DApp->GetShaderManager()->Load(pixel_name, L"main", ShaderManager::ShaderType::st_pixel);

}

Level::Level() :
    m_camera(std::make_unique<FreeCamera>())
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

        LevelEntity lev_ent{ L"", nullptr, pos, rot, scale, i };
        lev_ent.Load(model_name);
        m_entites.push_back(lev_ent);
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

            const LevelLight level_light{ DirectX::XMFLOAT3(), dir, power, ltype };
            m_lights.push_back(level_light);
        }
    }
}

const std::filesystem::path& Level::GetLevelsDir() const{
    return m_levels_dir;
}

const std::filesystem::path& Level::GetEntitiesDir() const{
    return m_entities_dir;
}