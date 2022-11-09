#pragma once

#include "free_allocator.h"

class GpuDataManager {
public:
    uint64_t AllocateVertexBuffer(uint32_t size);
    void DeallocateVertexBuffer(uint64_t start, uint32_t size);

private:
    static const uint32_t vertex_storage_size = 1024 * 1024 * 32;
    pro_game_containers::free_allocator m_vertex_storage{vertex_storage_size};
};