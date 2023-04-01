#pragma once

#include "IDescriptorHeapCollection.h"
#include <cassert>
#include <directx/d3dx12.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;


class DescriptorHeapCollection : public IDescriptorHeapCollection {
public:
    void Initialize(std::optional<std::wstring> dbg_name = std::nullopt) override;

    void ReserveRTVhandle(CPUdescriptor &rtvHandle) override {
        CD3DX12_CPU_DESCRIPTOR_HANDLE hndl;
        assert(m_rtv_actual_size < rtvHeap_size);
        hndl = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        hndl.Offset(m_rtv_actual_size++, m_rtvDescriptorSize);

        rtvHandle.ptr = hndl.ptr;
    }
    
    void ReserveDSVhandle(CPUdescriptor &dsvHandle) override {
        CD3DX12_CPU_DESCRIPTOR_HANDLE hndl;
        assert(m_dsv_actual_size < dsvHeap_size);
        hndl = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
        hndl.Offset(m_dsv_actual_size++, m_dsvDescriptorSize);

        dsvHandle.ptr = hndl.ptr;
    }

    void ReserveSRVUAVCBVhandle(CPUdescriptor &srvuacbvHandle) override {
        CD3DX12_CPU_DESCRIPTOR_HANDLE hndl;
            assert(m_srv_uav_actual_size < srvUavCbvHeap_size);
            hndl = m_srvUavCbvHeap->GetCPUDescriptorHandleForHeapStart();
            hndl.Offset(m_srv_uav_actual_size++, m_srvUavCbvDescriptorSize);

            srvuacbvHandle.ptr = hndl.ptr;
    }

private:
    static const uint32_t rtvHeap_size = 32;
    static const uint32_t dsvHeap_size = 5;
    static const uint32_t srvUavCbvHeap_size = 256;
    static const uint32_t srvUavCbvHeap_visible_size = 8;

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