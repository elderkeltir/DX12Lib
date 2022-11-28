#include "DynamicGpuHeap.h"
#include <string>
#include "RootSignature.h"

#include "DXAppImplementation.h"
#include "DXHelper.h"

extern DXAppImplementation* gD3DApp;

void DynamicGpuHeap::Initialize(uint32_t frame_id) {
	D3D12_DESCRIPTOR_HEAP_DESC srvUavCbvHeapDesc = {};
	srvUavCbvHeapDesc.NumDescriptors = HeapSize;
	srvUavCbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvUavCbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(gD3DApp->GetDevice()->CreateDescriptorHeap(&srvUavCbvHeapDesc, IID_PPV_ARGS(&m_visible_heap)));
	SetName(m_visible_heap, std::wstring(L"shader_vis_heap_").append(std::to_wstring(frame_id)).c_str());

    m_desciptor_size = gD3DApp->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DynamicGpuHeap::CacheRootSignature(const RootSignature * root_sig) {
    const auto &root_params_vec = root_sig->GetRootParams();
    m_tables_mask = 0;

    m_dirty_table_mask.resize(root_params_vec.size());
    for (uint32_t root_id = 0; root_id < root_params_vec.size(); root_id++){
        const CD3DX12_ROOT_PARAMETER1 &param = root_params_vec[root_id];
        if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE){
            m_tables_mask |= (1ull << root_id);

            uint32_t table_size = 0;
            for (uint32_t range_id = 0; range_id < param.DescriptorTable.NumDescriptorRanges; range_id++) {
                table_size+= param.DescriptorTable.pDescriptorRanges[range_id].NumDescriptors;
                m_dirty_table_mask[root_id] = 0;
            }

            m_root_sig_cache[root_id].num = table_size;

			CD3DX12_CPU_DESCRIPTOR_HANDLE begin_handle_cpu = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_visible_heap->GetCPUDescriptorHandleForHeapStart();
			begin_handle_cpu.Offset(m_actual_heap_size, m_desciptor_size);
			CD3DX12_GPU_DESCRIPTOR_HANDLE begin_handle_gpu = (CD3DX12_GPU_DESCRIPTOR_HANDLE)m_visible_heap->GetGPUDescriptorHandleForHeapStart();
            begin_handle_gpu.Offset(m_actual_heap_size, m_desciptor_size);
            m_root_sig_cache[root_id].base_cpu = begin_handle_cpu;
            m_root_sig_cache[root_id].base_gpu = begin_handle_gpu;
            m_root_sig_cache[root_id].staged.resize(table_size);
            m_actual_heap_size += table_size;
        }
    }
}

void DynamicGpuHeap::StageDesctriptor(uint32_t root_id, uint32_t offset, CD3DX12_CPU_DESCRIPTOR_HANDLE &desc_handle)
{
	m_root_sig_cache[root_id].staged[offset] = desc_handle;
	m_dirty_table_mask[root_id] |= (1ull << offset);
}

void DynamicGpuHeap::CommitRootSignature(ComPtr<ID3D12GraphicsCommandList6>& command_list, bool gfx) {
    for (uint32_t root_id = 0; root_id < MaxTableSize; root_id++) {
        if (m_tables_mask & (1ull << root_id)) {
            for (uint32_t i = 0; i < m_root_sig_cache[root_id].num; i++) {
				if (m_dirty_table_mask[root_id] & (1ull << i)) {
					CD3DX12_CPU_DESCRIPTOR_HANDLE begin_handle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_root_sig_cache[root_id].base_cpu;
					begin_handle.Offset(i, m_desciptor_size);

                    gD3DApp->GetDevice()->CopyDescriptorsSimple(1, begin_handle, m_root_sig_cache[root_id].staged[i], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    m_dirty_table_mask[root_id] &= (~(1ull << i));
				}
            }

		    CD3DX12_GPU_DESCRIPTOR_HANDLE begin_handle = (CD3DX12_GPU_DESCRIPTOR_HANDLE)m_root_sig_cache[root_id].base_gpu;
            if (gfx) {
                command_list->SetGraphicsRootDescriptorTable(root_id, begin_handle);
            }
            else {
                command_list->SetComputeRootDescriptorTable(root_id, begin_handle);
            }
        }
    }
}