#pragma once

#include <array>
#include <DirectXMath.h>
#include "LevelLight.h"
#include "IGpuResource.h"

class ICommandList;

enum class Constants {
    cCP,                    // camera pos
    cCD,                    // camera dir
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





class ConstantBufferManager {
public:
    void OnInit();
    void Destroy();
    void SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix);
    void SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix);
    void SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec);
    void SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec);
    void SetUint32(Constants id, uint32_t val);
    void SetModelCB(IGpuResource* res) { m_model_cb = res; }
    void CommitCB(ICommandList* command_list, ConstantBuffers id, bool gfx = true);

    static void SyncCpuDataToCB(ICommandList* command_list, IGpuResource* res, void* cpu_data, uint32_t size, BindingId bind_point, bool gfx = true);

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
        DirectX::XMFLOAT4 CamDir;
        DirectX::XMFLOAT4X4 VPinv;
        DirectX::XMFLOAT4 RTdim; // x=width, y=height, z=1/wisth, w=1/height
        DirectX::XMFLOAT4 NearFarZ; // x - Znear, y - Zfar, zw - free
        DirectX::XMFLOAT4 Time; // x - dt, y - total, zw - free
        // 256 x 1
        DirectX::XMFLOAT4X4 SunV;
        DirectX::XMFLOAT4X4 SunP;
    };

    IGpuResource* m_model_cb;
    std::array<std::unique_ptr<IGpuResource>, 2> m_scene_cbs;
};