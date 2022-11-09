#include "GpuDataManager.h"

uint64_t GpuDataManager::AllocateVertexBuffer(uint32_t size) {
    return m_vertex_storage.Allocate(size);
}

void GpuDataManager::DeallocateVertexBuffer(uint64_t start, uint32_t size) {
    m_vertex_storage.Deallocate(start, size);
}