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
#include "GpuResource.h"
#include "DXHelper.h"
#include "Techniques.h"
#include "DescriptorHeapCollection.h"
#include "ResourceDescriptor.h"
#include "MaterialManager.h"
#include "GfxCommandQueue.h"

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

        LevelEntity lev_ent{ pos, rot, scale };
        lev_ent.SetPos(pos);
        lev_ent.SetRot(rot);
        lev_ent.SetScale(scale);
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
            const Value& light_color = light["color"];
            // const DirectX::XMFLOAT3 pos(light_pos[0].GetFloat(), light_pos[1].GetFloat(), light_pos[2].GetFloat());
            const DirectX::XMFLOAT3 dir(light_dir[0].GetFloat(), light_dir[1].GetFloat(), light_dir[2].GetFloat());
            const DirectX::XMFLOAT3 color(light_color[0].GetFloat(), light_color[1].GetFloat(), light_color[2].GetFloat());

            const LevelLight level_light{ dir, color, ltype };
            uint32_t id = m_lights.push_back(level_light);
            m_lights[id].id = id;
        }
    }

    m_lights_res = std::make_unique<GpuResource>();
    uint32_t cb_size = (lights_num * sizeof(LevelLight) + 255) & ~255;
    m_lights_res->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"lights_buffer_").append(m_name));
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
    desc.SizeInBytes = cb_size;
    m_lights_res->Create_CBV(desc);
}

void Level::Update(float dt){
    // update camera
    m_camera->Update(dt);

    //update entites
    for (auto &entity : m_entites){
        entity.Update(dt);
    }
}

void Level::Render(ComPtr<ID3D12GraphicsCommandList6>& command_list){
    TODO("Normal! Create Gatherer or RenderScene to avoid this shity code")
    bool is_scene_constants_set = false;
    for (uint32_t id = 0; id < m_entites.size(); id++){
        LevelEntity &ent = m_entites[id];
        ent.LoadDataToGpu(command_list);

        const Techniques::Technique *tech = gD3DApp->GetTechniqueById(ent.GetTechniqueId());

        // set technique
        command_list->SetPipelineState(tech->pipeline_state.Get());
        command_list->SetGraphicsRootSignature(tech->root_signature.Get());

        // set root desc
        if (gD3DApp->ShouldMapHead(tech->id)){
            if (std::shared_ptr<DescriptorHeapCollection> descriptor_heap_collection = gD3DApp->GetDescriptorHeapCollection().lock()){
                ID3D12DescriptorHeap* descriptorHeaps[] = { descriptor_heap_collection->GetShaderVisibleHeap().Get() };
                command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
            }

            // update CBs
            if (std::shared_ptr<HeapBuffer> buff = m_lights_res->GetBuffer().lock()){
                if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
                    queue->ResourceBarrier(*m_lights_res.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
                }
                uint32_t cb_size = (lights_num * sizeof(LevelLight) + 255) & ~255;
                buff->Load(command_list, 1, cb_size, m_lights.data());
                if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
                    queue->ResourceBarrier(*m_lights_res.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
                }
            }

            if (std::shared_ptr<MaterialManager> mat_mgr = gD3DApp->GetMaterialManager().lock()){
                mat_mgr->SyncGpuData(command_list);
            }

            if (std::shared_ptr<ResourceDescriptor> srv = m_lights_res->GetCBV().lock()){
                command_list->SetGraphicsRootDescriptorTable(6, srv->GetGPUhandle());
            }
        }

        if (!is_scene_constants_set){
            DirectX::XMMATRIX m_ViewMatrix = DirectX::XMLoadFloat4x4(&m_camera->GetViewMx());
            DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMLoadFloat4x4(&m_camera->GetProjMx());
            TODO("Critical! Implement processing of different root desc setup")
            gD3DApp->SetMatrix4Constant(Constants::cV, m_ViewMatrix, command_list);
            gD3DApp->SetMatrix4Constant(Constants::cP, m_ProjectionMatrix, command_list);

            is_scene_constants_set = !is_scene_constants_set;
        }

        ent.Render(command_list);
    }
}

const std::filesystem::path& Level::GetLevelsDir() const{
    return m_levels_dir;
}

const std::filesystem::path& Level::GetEntitiesDir() const{
    return m_entities_dir;
}