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
#include "SkyBox.h"
#include "DynamicGpuHeap.h"
#include "Plane.h"

extern DXAppImplementation *gD3DApp;
using rapidjson::Document;
using rapidjson::Value;

Level::Level()
{
    m_levels_dir = gD3DApp->GetRootDir() / L"content" / L"levels";
    m_entities_dir = gD3DApp->GetRootDir() / L"content" / L"entities";
}
Level::~Level() = default;

void Level::Load(const std::wstring& name) {
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
        const Value& camera = d["camera"];
        const Value& camera_pos = camera["pos"];
        const Value& camera_dir = camera["dir"];
        const DirectX::XMFLOAT3 pos(camera_pos[0].GetFloat(), camera_pos[1].GetFloat(), camera_pos[2].GetFloat());
        const DirectX::XMFLOAT3 dir(camera_dir[0].GetFloat(), camera_dir[1].GetFloat(), camera_dir[2].GetFloat());

        const Value& camera_fov = camera["fov"];
        const Value& camera_near = camera["near"];
        const Value& camera_far = camera["far"];

        m_camera.swap(std::make_shared<FreeCamera>(camera_fov.GetFloat(), camera_near.GetFloat(), camera_far.GetFloat(), gD3DApp->GetAspectRatio()));
        m_camera->Move(pos);
        m_camera->Rotate(dir);
    }

    // entities
    {
        const Value& entities = d["entities"];
        for (uint32_t i = 0; i < entities.Size(); i++) {
            const Value& entity = entities[i];
            const char* model_name_8 = entity["model"].GetString();
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
    }

    // Lights
    {
        const Value& lights = d["lights"];
        for (uint32_t i = 0; i < lights.Size(); i++) {
            const Value& light = lights[i];
            const LevelLight::LightType ltype = static_cast<LevelLight::LightType>(light["type"].GetInt());

            if (ltype == LevelLight::LightType::lt_direct) {
                const Value& light_dir = light["dir"];
                const Value& light_color = light["color"];
                const DirectX::XMFLOAT3 dir(light_dir[0].GetFloat(), light_dir[1].GetFloat(), light_dir[2].GetFloat());
                const DirectX::XMFLOAT3 color(light_color[0].GetFloat(), light_color[1].GetFloat(), light_color[2].GetFloat());

                LevelLight level_light;
                level_light.type = ltype;
                level_light.color = color;
                level_light.dir = dir;
                uint32_t id = m_lights.push_back(level_light);
                m_lights[id].id = id;
            }
            else if (ltype == LevelLight::LightType::lt_point) {
                const Value& light_pos = light["pos"];
                const Value& light_color = light["color"];
                const DirectX::XMFLOAT3 pos(light_pos[0].GetFloat(), light_pos[1].GetFloat(), light_pos[2].GetFloat());
                const DirectX::XMFLOAT3 color(light_color[0].GetFloat(), light_color[1].GetFloat(), light_color[2].GetFloat());

                LevelLight level_light;
                level_light.type = ltype;
                level_light.color = color;
                level_light.pos = pos;
                uint32_t id = m_lights.push_back(level_light);
                m_lights[id].id = id;
            }
        }

        m_lights_res = std::make_unique<GpuResource>();
        uint32_t cb_size = calc_cb_size(LightsNum * sizeof(LevelLight));
        m_lights_res->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"lights_buffer_").append(m_name));
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.SizeInBytes = cb_size;
        m_lights_res->Create_CBV(desc);
    }

    // Skybox
    {
        const Value& skybox = d["skybox"];
        const char* skybox_name_8 = skybox["entity"].GetString();
        const std::wstring skybox_name(&skybox_name_8[0], &skybox_name_8[strlen(skybox_name_8)]);
        m_skybox_ent.swap(std::make_unique<SkyBox>());
        m_skybox_ent->Load(skybox_name);
    }

    // Terrain
    {
        const Value& terrain = d["terrain"];
        const char* terrain_hm_name_8 = terrain["height_map"].GetString();
        const std::wstring terrain_hm_name(&terrain_hm_name_8[0], &terrain_hm_name_8[strlen(terrain_hm_name_8)]);
        const Value& terrain_pos = terrain["pos"];
        const DirectX::XMFLOAT4 pos(terrain_pos[0].GetFloat(), terrain_pos[1].GetFloat(), terrain_pos[2].GetFloat(), 1);
        const uint32_t terrain_dim = terrain["dim"].GetUint();
        const uint32_t terrain_tech_id = terrain["tech_id"].GetUint();

        m_terrain.swap(std::make_unique<Plane>());
        m_terrain->Load(terrain_hm_name, terrain_dim, terrain_tech_id, pos);
    }

	// Water
    {
        const Value& water = d["water"];
        const uint32_t water_dim = water["dim"].GetUint();
        const uint32_t water_tech_id = water["tech_id"].GetUint();
		const Value& water_pos = water["pos"];
		const DirectX::XMFLOAT4 pos(water_pos[0].GetFloat(), water_pos[1].GetFloat(), water_pos[2].GetFloat(), 1);

        m_water.swap(std::make_unique<Plane>());
        m_water->Load(L"", water_dim, water_tech_id, pos);
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

void Level::Render(ComPtr<ID3D12GraphicsCommandList6>& command_list){
    TODO("Normal! Create Gatherer or RenderScene to avoid this shity code")
    bool is_scene_constants_set = false;

    for (uint32_t id = 0; id < m_entites.size(); id++) {
        LevelEntity &ent = m_entites[id];
        RenderEntity(command_list, ent, is_scene_constants_set);
    }

    RenderEntity(command_list, *m_skybox_ent, is_scene_constants_set);
    {
        uint32_t terrain_tech_id = m_terrain->GetTerrainTechId();
        const Techniques::Technique* tech = gD3DApp->GetTechniqueById(terrain_tech_id);
        if (std::shared_ptr<GfxCommandQueue> gfx_queue = gD3DApp->GetGfxQueue().lock()) {
            if (gfx_queue->GetPSO() != terrain_tech_id) {
                gfx_queue->SetPSO(terrain_tech_id);
            }
            if (gfx_queue->GetRootSign() != tech->root_signature) {
                gfx_queue->SetRootSign(tech->root_signature);
            }
            gfx_queue->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
        }
        gD3DApp->CommitCB(command_list, cb_scene);
        m_terrain->Render(command_list);
    }
}

void Level::RenderEntity(ComPtr<ID3D12GraphicsCommandList6>& command_list, LevelEntity & ent, bool &is_scene_constants_set){
    ent.LoadDataToGpu(command_list);

    const Techniques::Technique *tech = gD3DApp->GetTechniqueById(ent.GetTechniqueId());
    if (std::shared_ptr<GfxCommandQueue> gfx_queue = gD3DApp->GetGfxQueue().lock()){
        if (gfx_queue->GetPSO() != ent.GetTechniqueId()){
            gfx_queue->SetPSO(ent.GetTechniqueId());
        }
        if (gfx_queue->GetRootSign() != tech->root_signature){
            gfx_queue->SetRootSign(tech->root_signature);
        }
        gfx_queue->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
    }

    if (!is_scene_constants_set){
        gD3DApp->CommitCB(command_list, cb_scene);
        DirectX::XMMATRIX m_ViewMatrix = DirectX::XMLoadFloat4x4(&m_camera->GetViewMx());
        DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMLoadFloat4x4(&m_camera->GetProjMx());

        gD3DApp->SetMatrix4Constant(Constants::cV, m_ViewMatrix);
        gD3DApp->SetMatrix4Constant(Constants::cP, m_ProjectionMatrix);

        DirectX::XMMATRIX Pinv = DirectX::XMMatrixInverse(nullptr, m_ProjectionMatrix);
        gD3DApp->SetMatrix4Constant(Constants::cPinv, Pinv);

        DirectX::XMFLOAT4 cam_pos(m_camera->GetPosition().x, m_camera->GetPosition().y, m_camera->GetPosition().z, 1);
        gD3DApp->SetVector4Constant(Constants::cCP, cam_pos);

        float w = (float)gD3DApp->GetWidth();
        float h = (float)gD3DApp->GetHeight();
        DirectX::XMFLOAT4 rt_dim(w, h, 1.f / w, 1.f / h);
        gD3DApp->SetVector4Constant(Constants::cRTdim, rt_dim);

        DirectX::XMFLOAT4 z_near_far(m_camera->GetNearZ(), m_camera->GetFarZ(), (float)m_terrain->GetTerrainDim(), (float)gD3DApp->GetRenderMode());
        gD3DApp->SetVector4Constant(Constants::cNearFar, z_near_far);

        DirectX::XMFLOAT4 time_vec(gD3DApp->FrameTime().count(), gD3DApp->TotalTime().count(), 0, 0);
        gD3DApp->SetVector4Constant(Constants::cTime, time_vec);
		
        // update material CBs
		if (std::shared_ptr<MaterialManager> mat_mgr = gD3DApp->GetMaterialManager().lock()) {
			mat_mgr->BindMaterials(command_list);
		}

        is_scene_constants_set = !is_scene_constants_set;
    }

    ent.Render(command_list);
}

void Level::RenderWater(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
    uint32_t tech_id = m_water->GetTerrainTechId();
	const Techniques::Technique* tech = gD3DApp->GetTechniqueById(tech_id);
    if (std::shared_ptr<GfxCommandQueue> gfx_queue = gD3DApp->GetGfxQueue().lock()) {
        if (gfx_queue->GetPSO() != tech_id) {
            gfx_queue->SetPSO(tech_id);
        }
        if (gfx_queue->GetRootSign() != tech->root_signature) {
            gfx_queue->SetRootSign(tech->root_signature);
        }
        
        gfx_queue->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));

        GpuResource* skybox_tex = m_skybox_ent->GetTexture();
		if (std::shared_ptr<ResourceDescriptor> srv = skybox_tex->GetSRV().lock()) {
			gfx_queue->GetGpuHeap().StageDesctriptor(bi_fwd_tex, tto_fwd_skybox, srv->GetCPUhandle());
		}
        gD3DApp->CommitCB(command_list, cb_scene);
        BindLights(command_list);
    }

    m_water->Render(command_list);
}

void Level::BindLights(ComPtr<ID3D12GraphicsCommandList6>& command_list){
    ConstantBufferManager::SyncCpuDataToCB(command_list, m_lights_res.get(), m_lights.data(), (LightsNum * sizeof(LevelLight)), bi_lights_cb);
}

const std::filesystem::path& Level::GetLevelsDir() const{
    return m_levels_dir;
}

const std::filesystem::path& Level::GetEntitiesDir() const{
    return m_entities_dir;
}