#pragma once

#include "IDynamicGpuHeap.h"
#include <directx/d3dx12.h>
#include <wrl.h>
#include <vector>
struct CPUdescriptor;
struct GPUdescriptor;
class IRootSignature;

using Microsoft::WRL::ComPtr;

class RootSignature;
class ICommandList;
class IResourceDescriptor;

class DynamicGpuHeap : public IDynamicGpuHeap {
public:
    void Initialize(uint32_t frame_id) override;
    void CacheRootSignature(const IRootSignature* root_sig, uint32_t tech_id = 0) override;
    void StageDesctriptorInTable(uint32_t root_id, uint32_t offset, const std::shared_ptr<IResourceDescriptor>& desc_handle) override;
    void ReserveDescriptor(CPUdescriptor& cpu_descriptor, GPUdescriptor& gpu_descriptor) override;
    void CommitRootSignature(ICommandList* command_list, bool gfx = true) override;
    const ComPtr<ID3D12DescriptorHeap>& GetVisibleHeap() const { return m_visible_heap; }
    void Reset() override {
        m_actual_heap_size = 0;
        m_tables_mask ^= m_tables_mask;
        m_dirty_table_mask.clear();
    }

    ~DynamicGpuHeap();
private:
    struct TableCache {
		uint32_t num;
		D3D12_CPU_DESCRIPTOR_HANDLE base_cpu; // TODO: switch to user-defined types here
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