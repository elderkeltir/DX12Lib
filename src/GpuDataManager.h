#pragma once

#include <memory>
#include "free_allocator.h"

class IGpuResource;
class ICommandList;

class GpuDataManager {
public:
    uint64_t AllocateVertexBuffer(uint32_t size);
    void DeallocateVertexBuffer(uint64_t start, uint32_t size);

    void Initialize();
    void UploadToGpu(ICommandList* command_list);
    IGpuResource* GetVertexBuffer() { return m_vertex_buffer_res.get(); }
    uint64_t GetBase() const { return m_vertex_storage.begin(); }

private:
    static const uint32_t vertex_storage_size = 1024 * 1024 * 32;
    pro_game_containers::free_allocator m_vertex_storage{vertex_storage_size};
    std::unique_ptr<IGpuResource> m_vertex_buffer_res;
    bool m_dirty{ false };
};