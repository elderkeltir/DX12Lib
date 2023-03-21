#pragma once

#include <array>
#include <DirectXMath.h>
#include <directx/d3d12.h>
#include <wrl.h>
#include "LevelLight.h"
#include "GpuResource.h"

using Microsoft::WRL::ComPtr;

class CommandList;

enum class Constants {
    cCP,                    // camera pos
    cM,                     // model matrix
    cV,                     // view matrix
    cP,                     // projection matrix
    cMat,                   // material id
    cPinv,                  // ViewProj inverted
    cRTdim,                 // rt size
    cNearFar,               // x - Znear, y - Zfar, z - terrain_dim, w - render_mode (0 - default, 1 - ssao, 2 - sun sm)
    cTime,                  // x - dt, y - total_time, zw - FREE
    cVertexBufferOffset,    // offset in bindless vertex buffer
    cVertexType,            // vertex type for render model
    cSunV,                  // sun V mx
    cSunP,                  // sun P mx
};

enum BindingId {
    bi_model_cb                     = 0,
    bi_g_buffer_tex_table           = 1,
    bi_scene_cb                     = 2,
    bi_vertex_buffer                = 4,
    bi_materials_cb                 = 3,
    bi_lights_cb                    = 3,
    bi_deferred_shading_tex_table   = 1,
    bi_post_proc_input_tex_table    = 0,
    bi_ssao_cb                      = 0,
    bi_ssao_input_tex               = 1,
    bi_ssao_uav_tex                 = 3,
    bi_terrain_hm                   = 1,
    bi_fwd_tex                      = 1,
};

enum TextureTableOffset {
    tto_albedo                      = 0,
    tto_normals                     = 1,
    tto_metallic                    = 2,
    tto_roughness                   = 3,
    tto_gbuff_albedo                = 0,
    tto_gbuff_normals               = 1,
    tto_gbuff_positions             = 2,
    tto_gbuff_materials             = 3,
    tto_gbuff_ssao                  = 4,
    tto_gbuff_sun_sm                = 5,
    tto_postp_input                 = 0,
    tto_postp_gui                   = 1,
    tto_postp_fwd                   = 2,
    tto_postp_ssao                  = 3,
    tto_postp_sun_sm                = 4,
    tto_ssao_depth                  = 0,
    tto_ssao_normals                = 1,
    tto_ssao_random_vals            = 2,
    tto_ssao_positions              = 3,
    tto_ssao_blur_srv               = 0,
    tto_ssao_blur_uav               = 0,
    tto_fwd_skybox                  = 0,
    tto_vertex_buffer               = 5,
};

enum ConstantBuffers {
    cb_model                        = 0,
    cb_scene                        = 1,
    cb_lights                       = 2,
    cb_materials                    = 3,
    cb_ssao                         = 4,
};

class ConstantBufferManager {
public:
    void OnInit();
    void Destroy();
    void SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix);
    void SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix);
    void SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec);
    void SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec);
    void SetUint32(Constants id, uint32_t val);
    void SetModelCB(GpuResource* res) { m_model_cb = res; }
    void CommitCB(CommandList& command_list, ConstantBuffers id, bool gfx = true);

    static void SyncCpuDataToCB(CommandList& command_list, GpuResource* res, void* cpu_data, uint32_t size, BindingId bind_point, bool gfx = true);

public:
    // 1 x 256
    struct ModelCB {
        DirectX::XMFLOAT4X4 M;
        uint32_t material_id;
        uint32_t vertex_buffer_offset;
        uint32_t vertex_type;
        float padding;
    };

    // 2 x 256
    struct SceneCB {
        DirectX::XMFLOAT4X4 V;
        DirectX::XMFLOAT4X4 P;
        DirectX::XMFLOAT4 CamPos;
        DirectX::XMFLOAT4X4 VPinv;
        DirectX::XMFLOAT4 RTdim; // x=width, y=height, z=1/wisth, w=1/height
        DirectX::XMFLOAT4 NearFarZ; // x - Znear, y - Zfar, zw - free
        DirectX::XMFLOAT4 Time; // x - dt, y - total, zw - free
        // 256 x 1
        DirectX::XMFLOAT4X4 SunV;
        DirectX::XMFLOAT4X4 SunP;
    };

    GpuResource* m_model_cb;
    std::array<GpuResource, 2> m_scene_cbs;
};