#include "DescriptorHeapCollection.h"

#include "DXAppImplementation.h"
#include "DXHelper.h"

extern DXAppImplementation *gD3DApp;

void DescriptorHeapCollection::Initialize(std::optional<std::wstring> dbg_name = std::nullopt){
    TODO("Normal! make some intelegent mechanism to take care of descriptors allocations. later.")
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = rtvHeap_size;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(gD3DApp->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    SetName(m_commandAllocator, dbg_name.value_or(wstring_empty).append("_rtv_heap").c_str());
    
    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = dsvHeap_size;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(gD3DApp->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    SetName(m_commandAllocator, dbg_name.value_or(wstring_empty).append("_dsv_heap").c_str());
    
    // Describe and create a shader resource view (SRV) and unordered
    // access view (UAV) descriptor heap.
    TODO("Critical! Create separate heap with shader visibility and copy there")
    D3D12_DESCRIPTOR_HEAP_DESC srvUavCbvHeapDesc = {};
    srvUavCbvHeapDesc.NumDescriptors = srvUavCbvHeap_size;
    srvUavCbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvUavCbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(gD3DApp->GetDevice()->CreateDescriptorHeap(&srvUavCbvHeapDesc, IID_PPV_ARGS(&m_srvUavCbvHeap)));
    SetName(m_commandAllocator, dbg_name.value_or(wstring_empty).append("_srv_heap, shader_visibility=")append(std::to_string(srvUavCbvHeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)).c_str());
    
    m_rtvDescriptorSize = gD3DApp->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = gD3DApp->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_srvUavCbvDescriptorSize = gD3DApp->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}