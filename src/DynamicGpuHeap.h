#pragma once

#include <directx/d3dx12.h>
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class RootSignature;
class CommandList;

class DynamicGpuHeap {
public:
    enum ResourcesIdx { 
        ri_albedo,
        ri_normal
    };
    void Initialize(uint32_t frame_id);
    void CacheRootSignature(const RootSignature * root_sig);
    void StageDesctriptorInTable(uint32_t root_id, uint32_t offset, CD3DX12_CPU_DESCRIPTOR_HANDLE& desc_handle);

    void CommitRootSignature(CommandList& command_list, bool gfx = true);
    ComPtr<ID3D12DescriptorHeap>& GetVisibleHeap() { return m_visible_heap; }
    void Reset() { 
        m_actual_heap_size = 0;
        m_tables_mask ^= m_tables_mask;
        m_dirty_table_mask.clear();
    }
private:
    struct TableCache {
		uint32_t num;
		D3D12_CPU_DESCRIPTOR_HANDLE base_cpu;
        D3D12_GPU_DESCRIPTOR_HANDLE base_gpu;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> staged;
    };
    static const uint32_t HeapSize = 128;
    static const uint32_t MaxTableSize = 32;

    ComPtr<ID3D12DescriptorHeap> m_visible_heap;
    TableCache m_root_sig_cache[MaxTableSize];
    uint64_t m_tables_mask{0};
    std::vector<uint64_t> m_dirty_table_mask{ 0 };

    uint32_t m_actual_heap_size{ 0 };
    uint32_t m_desciptor_size{ 0 };
};