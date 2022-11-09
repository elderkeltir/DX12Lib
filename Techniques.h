#pragma once
#include <optional>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <directx/d3d12.h>
#include <wrl.h>

#include "simple_object_pool.h"

using Microsoft::WRL::ComPtr;

class Techniques {
public:
    struct Technique {
        ComPtr<ID3D12PipelineState> pipeline_state;
        ComPtr<ID3D12RootSignature> root_signature;
        std::wstring vs;
        std::wstring ps;
        uint32_t vertex_type;
        uint32_t id;
    };
public:
    virtual void OnInit(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt);
    const Techniques::Technique * GetTechniqueById(uint32_t id) const { return &m_techniques[id]; }

private:
 
    static constexpr uint32_t TechniquesCount = 8;
    pro_game_containers::simple_object_pool<Technique, TechniquesCount> m_techniques;
};