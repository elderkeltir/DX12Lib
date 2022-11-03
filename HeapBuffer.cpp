#include "HeapBuffer.h"

#include "DXHelper.h"
#include <directx/d3d12.h>
#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

void HeapBuffer::Create(BufferType type, uint32_t bufferSize, UseFlag flags, D3D12_RESOURCE_STATES initial_state, std::optional<std::wstring> dbg_name) {
    D3D12_HEAP_TYPE internal_type;
    D3D12_RESOURCE_FLAGS internal_flags;

    switch(type){
        case BufferType::bt_default:
            internal_type = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case BufferType::bt_upload:
            internal_type = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case BufferType::bt_readback:
            internal_type = D3D12_HEAP_TYPE_READBACK;
            break;
    };

    switch(flags){
        case UseFlag::uf_none:
            internal_flags = D3D12_RESOURCE_FLAG_NONE;
            break;
        case UseFlag::uf_rt:
            internal_flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            break;
        case UseFlag::uf_ds:
            internal_flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            break;
        case UseFlag::uf_srv:
            internal_flags = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            break;
        case UseFlag::uf_uav:
            internal_flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            break;
    };

    ThrowIfFailed(gD3DApp->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(internal_type),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, internal_flags),
        initial_state,
        nullptr,
        IID_PPV_ARGS(&m_resourse)));
    SetName(m_resourse, dbg_name.value_or(L"").append(L"_buffer").c_str());
    
    m_recreate_intermediate_res = true;
}

void HeapBuffer::CreateTexture(BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE *clear_val, std::optional<std::wstring> dbg_name){
    D3D12_HEAP_TYPE internal_type;

    switch(type){
        case BufferType::bt_default:
            internal_type = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case BufferType::bt_upload:
            internal_type = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case BufferType::bt_readback:
            internal_type = D3D12_HEAP_TYPE_READBACK;
            break;
    };

    ThrowIfFailed(gD3DApp->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(internal_type),
        D3D12_HEAP_FLAG_NONE,
        &res_desc,
        initial_state,
        clear_val,
        IID_PPV_ARGS(&m_resourse)
    ));
    SetName(m_resourse, dbg_name.value_or(L"").append(L"_texture").c_str());
    
    m_recreate_intermediate_res = true;
}

void HeapBuffer::Load(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t numElements, uint32_t elementSize, const void* bufferData){
    if (bufferData)
    {
        auto& device = gD3DApp->GetDevice();
        const size_t bufferSize = numElements * elementSize;
        if (m_recreate_intermediate_res)
        {
            pIntermediateResource.Reset();
            ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(pIntermediateResource.GetAddressOf())));
                SetName(pIntermediateResource, L"_intermediate");
            
            m_recreate_intermediate_res = !m_recreate_intermediate_res;
        }

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            m_resourse.Get(), pIntermediateResource.Get(),
            0, 0, 1, &subresourceData);
    }
}

void HeapBuffer::Load(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData){
    if (subresourceData)
    {
        auto& device = gD3DApp->GetDevice();
        const uint64_t required_size = GetRequiredIntermediateSize(m_resourse.Get(), firstSubresource, numSubresources);
        if (m_recreate_intermediate_res)
        {
            pIntermediateResource.Reset();
            ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(required_size),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(pIntermediateResource.GetAddressOf())));
                SetName(pIntermediateResource, L"_intermediate");
            
            m_recreate_intermediate_res = !m_recreate_intermediate_res;
        }

        UpdateSubresources(commandList.Get(),
            m_resourse.Get(), pIntermediateResource.Get(),
            0, firstSubresource, numSubresources, subresourceData);
    }
}

BYTE* HeapBuffer::Map(){
    BYTE* pData;
    ThrowIfFailed(m_resourse->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

    return pData;
}

void HeapBuffer::Unmap(){
    m_resourse->Unmap(0, nullptr);
}

void HeapBuffer::Copy(ComPtr<ID3D12GraphicsCommandList> &commandList, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size){
    commandList->CopyBufferRegion(dest.m_resourse.Get(), dstOffset, m_resourse.Get(), srcOffset, size);
}