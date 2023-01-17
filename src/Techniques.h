#pragma once
#include <optional>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <directx/d3d12.h>
#include <wrl.h>

#include "simple_object_pool.h"
#include "RootSignature.h"

using Microsoft::WRL::ComPtr;

class Techniques {
public:
    struct Technique {
        ComPtr<ID3D12PipelineState> pipeline_state;
        uint32_t root_signature;
        std::wstring vs;
        std::wstring ps;
        std::wstring cs;
        uint32_t vertex_type;
        uint32_t id;
    };
    enum TecnhinueType {
        tt_post_processing                  = 2,
        tt_deferred_shading                 = 3,
        tt_ssao                             = 5,
        tt_blur                             = 6,
        tt_shadow_map                       = 9,
    };
public:
    virtual void OnInit(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt);
    const Techniques::Technique * GetTechniqueById(uint32_t id) const { return &m_techniques[id]; }
    const RootSignature * GetRootSignById(uint32_t id) const { return &m_root_signatures[id]; }
    bool TechHasColor(uint32_t tech_id);
    virtual void RebuildShaders(std::optional<std::wstring> dbg_name = std::nullopt);
private:
    void CreateRootSignature_0(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_1(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_2(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_3(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_4(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);

    void LoadTechniques();
    static constexpr uint32_t TechniquesCount = 16;
    static constexpr uint32_t RootSignCount = 16;
    static constexpr uint32_t DescRangeNum = 256;
    pro_game_containers::simple_object_pool<RootSignature, RootSignCount> m_root_signatures;
    pro_game_containers::simple_object_pool<Technique, TechniquesCount> m_techniques;
    pro_game_containers::simple_object_pool <CD3DX12_DESCRIPTOR_RANGE1, DescRangeNum> m_desc_ranges;
};