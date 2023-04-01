#pragma once

#include <string>
#include <optional>
#include <wrl.h>                // COM helpers.
using Microsoft::WRL::ComPtr;
struct ID3D12Device2;
struct ID3D12PipelineState;
class IRootSignature;


class ITechniques {
public:
    struct Technique {
        ComPtr<ID3D12PipelineState> pipeline_state; // TODO: get rid of it? or use ifdef
        uint32_t root_signature;
        std::wstring vs;
        std::wstring ps;
        std::wstring cs;
        uint32_t vertex_type;
        uint32_t id;
    };
    enum TecnhinueType {
        tt_post_processing = 2,
        tt_deferred_shading = 3,
        tt_ssao = 5,
        tt_blur = 6,
        tt_shadow_map = 9,
        tt_reflection_map = 10,
    };
public:
    virtual void OnInit(std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual const ITechniques::Technique* GetTechniqueById(uint32_t id) const = 0;
    virtual const IRootSignature* GetRootSignById(uint32_t id) const = 0;
    virtual bool TechHasColor(uint32_t tech_id) = 0;
    virtual void RebuildShaders(std::optional<std::wstring> dbg_name = std::nullopt) = 0;
};