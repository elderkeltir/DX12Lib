#pragma once

#include <cassert>
#include <directx/d3dx12.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class DescriptorHeapCollection {
public:
    void Initialize();

    void ReserveRTVhandle(CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle) { 
        assert(m_rtv_actual_size < rtvHeap_size);
        rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.Offset(m_rtv_actual_size++, m_rtvDescriptorSize);
    }
    
    void ReserveDSVhandle(CD3DX12_CPU_DESCRIPTOR_HANDLE &dsvHandle) { 
        assert(m_dsv_actual_size < dsvHeap_size);
        dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
        dsvHandle.Offset(m_dsv_actual_size++, m_dsvDescriptorSize);
    }

    void ReserveSRVUAVCBVhandle(CD3DX12_CPU_DESCRIPTOR_HANDLE &srvuacbvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE srvuacbvHandle_gpu) { 
        assert(m_srv_uav_actual_size < srvUavCbvHeap_size);
        srvuacbvHandle = m_srvUavCbvHeap->GetCPUDescriptorHandleForHeapStart();
        srvuacbvHandle.Offset(m_srv_uav_actual_size, m_srvUavCbvDescriptorSize);

        srvuacbvHandle_gpu = m_srvUavCbvHeap->GetGPUDescriptorHandleForHeapStart();
        srvuacbvHandle_gpu.Offset(m_srv_uav_actual_size++, m_srvUavCbvDescriptorSize);
    }


private:
    static const uint32_t rtvHeap_size = 2;
    static const uint32_t dsvHeap_size = 1;
    static const uint32_t srvUavCbvHeap_size = 10;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvUavCbvHeap;

    uint32_t m_rtvDescriptorSize;
    uint32_t m_dsvDescriptorSize;
    uint32_t m_srvUavCbvDescriptorSize;

    uint32_t m_rtv_actual_size{0};
    uint32_t m_dsv_actual_size{0};
    uint32_t m_srv_uav_actual_size{0};
};