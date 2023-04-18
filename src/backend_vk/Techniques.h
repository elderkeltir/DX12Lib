#pragma once
#include <optional>
#include <cstdint>
#include <string>

#include "ITechniques.h"
#include "simple_object_pool.h"
#include "RootSignature.h"

class Techniques : public ITechniques {
public:
    struct TechniqueVk : public ITechniques::Technique {
        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
        uint32_t rp_id;
    };
    struct RenderPass {
        VkRenderPass rpass;
        std::vector<VkFormat> formats;
        uint32_t id;
        bool depth;
    };
public:
    void OnInit(std::optional<std::wstring> dbg_name = std::nullopt) override;
    const ITechniques::Technique * GetTechniqueById(uint32_t id) const override { return (ITechniques::Technique*)&m_techniques[id]; }
    const IRootSignature* GetRootSignById(uint32_t id) const override { return &m_root_signatures[id]; }
    bool TechHasColor(uint32_t tech_id) override;
    void RebuildShaders(std::optional<std::wstring> dbg_name = std::nullopt) override;

    VkSampler GetSamplerById(uint32_t id) {
        return m_samplers[id];
    }

    const Techniques::RenderPass& GetRenderPassById(uint32_t id) const {
        return m_render_passes[id];
    }
private:
    void CreateRootSignature_0(RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_1(RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_2(RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_3(RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateRootSignature_4(RootSignature* root_sign, std::optional<std::wstring> dbg_name = std::nullopt);

    static constexpr uint32_t TechniquesCount = 16;
    static constexpr uint32_t RootSignCount = 16;
    static constexpr uint32_t RenderPassNum = 32;
    static constexpr uint32_t SamplersCount = 16;
    pro_game_containers::simple_object_pool<RootSignature, RootSignCount> m_root_signatures;
    pro_game_containers::simple_object_pool<TechniqueVk, TechniquesCount> m_techniques;
    pro_game_containers::simple_object_pool <RenderPass, RenderPassNum> m_render_passes;
    pro_game_containers::simple_object_pool <VkSampler, SamplersCount> m_samplers;
};