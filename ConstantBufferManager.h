#pragma once

#include <memory>
#include <DirectXMath.h>
#include <directx/d3d12.h>
#include <wrl.h>
#include "LevelLight.h"

using Microsoft::WRL::ComPtr;
class GpuResource;

enum class Constants {
    cCP,            // camera pos
    cM,             // model matrix
    cV,             // view matrix
    cP,             // projection matrix
    cMat            // material id
};

enum BindingId {
    bi_model_cb                     = 0,
    bi_g_buffer_tex_table           = 1,
    bi_scene_cb                     = 2,
    bi_materials_cb                 = 3,
    bi_lights_cb                    = 0,
    bi_deferred_shading_tex_table   = 1,
    bi_post_proc_input_tex_table    = 0
};

enum TextureTableOffset {
    tto_albedo                      = 0,
    tto_normals                     = 1,
    tto_metallic                    = 2,
    tto_gbuff_albedo                = 0,
    tto_gbuff_normals               = 1,
    tto_gbuff_positions             = 2,
    tto_gbuff_materials             = 3,
    tto_postp_input                 = 0
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
    void CommitCB(ComPtr<ID3D12GraphicsCommandList6>& command_list, uint32_t id);

public:
    // 1 x 256
    struct ModelCB {
        DirectX::XMFLOAT4X4 M;
        uint32_t material_id;
        DirectX::XMFLOAT3 padding;
    };

    // 1 x 256
    struct SceneCB {
        DirectX::XMFLOAT4X4 V;
        DirectX::XMFLOAT4X4 P;
        DirectX::XMFLOAT4 CamPos;
    };

    GpuResource* m_model_cb;
    std::unique_ptr<GpuResource> m_scene_cb;
};