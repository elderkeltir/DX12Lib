#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <cassert>
#include <directx/d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Techniques {
public:
    struct Technique {
        ComPtr<ID3D12PipelineState> pipeline_state;
        ComPtr<ID3D12RootSignature> root_signature;
        std::wstring vs;
        std::wstring ps;
    };
public:
    virtual void OnInit(ComPtr<ID3D12Device2> &device);
    const Techniques::Technique * GetTechniqueById(uint32_t id) const { assert(id < m_actual_techniques_count); return &m_techniques[id]; }

private:
 
    static constexpr uint32_t TechniquesCount = 8;
    uint32_t m_actual_techniques_count{0};
    std::array<Technique, TechniquesCount> m_techniques;
};