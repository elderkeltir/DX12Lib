#pragma once

#include "RenderObject.h"
#include <optional>

class ICommandList;

class RenderQuad : public RenderObject {
public:
    ~RenderQuad();
    void Initialize();

    bool CreateQuadTexture(uint32_t width, uint32_t height, const std::vector<ResourceFormat> &formats, uint32_t texture_num, uint32_t uavs, std::optional<std::wstring> dbg_name = std::nullopt);

    std::weak_ptr<IGpuResource> GetRt(uint32_t set_idx, uint32_t idx_in_set = 0u);
    std::vector< std::shared_ptr<IGpuResource>>& GetRts(uint32_t set_idx) { return m_textures.at(set_idx); }
    void Render(ICommandList* command_list);
private:

    std::vector<std::vector<std::shared_ptr<IGpuResource>>> m_textures;
};