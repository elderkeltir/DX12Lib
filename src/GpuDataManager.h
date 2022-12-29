#pragma once

#include <memory>
#include <directx/d3dx12.h>
#include "free_allocator.h"

using Microsoft::WRL::ComPtr;

class GpuResource;
class CommandList;

class GpuDataManager {
public:
    uint64_t AllocateVertexBuffer(uint32_t size);
    void DeallocateVertexBuffer(uint64_t start, uint32_t size);

    void Initialize(CommandList& command_list);
    void UploadToGpu(CommandList& command_list);
    GpuResource* GetVertexBuffer() { return m_vertex_buffer_res.get(); }

private:
    static const uint32_t vertex_storage_size = 1024 * 1024 * 32;
    pro_game_containers::free_allocator m_vertex_storage{vertex_storage_size};
    std::unique_ptr<GpuResource> m_vertex_buffer_res;
    bool m_dirty{ false };
};