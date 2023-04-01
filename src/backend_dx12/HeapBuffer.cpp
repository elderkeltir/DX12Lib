#include "HeapBuffer.h"

#include "dx12_helper.h"
#include <directx/d3d12.h>
#include "DxBackend.h"
#include "ICommandList.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

void HeapBuffer::Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name) {
    D3D12_HEAP_TYPE internal_type{ (D3D12_HEAP_TYPE)type };

    ThrowIfFailed(gBackend->GetDevice()->GetNativeObject()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(internal_type),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE),
        (D3D12_RESOURCE_STATES)initial_state,
        nullptr,
        IID_PPV_ARGS(&m_resourse)));
    SetName(m_resourse, dbg_name.value_or(L"").append(L"_buffer").c_str());

    m_recreate_intermediate_res = true;
}

void HeapBuffer::CreateTexture(HeapType type, const ResourceDesc &res_desc, ResourceState initial_state, const ClearColor *clear_val, std::optional<std::wstring> dbg_name){
    D3D12_HEAP_TYPE internal_type{ (D3D12_HEAP_TYPE)type };

    CD3DX12_RESOURCE_DESC res_desc_native;
    D3D12_CLEAR_VALUE clear_val_native;

    {
        res_desc_native.Alignment = res_desc.alignment;
        res_desc_native.DepthOrArraySize = res_desc.depth_or_array_size;
        res_desc_native.Dimension = (D3D12_RESOURCE_DIMENSION)res_desc.resource_dimension;
        res_desc_native.Flags = (D3D12_RESOURCE_FLAGS)res_desc.resource_flags;
        res_desc_native.Format = (DXGI_FORMAT)res_desc.format;
        res_desc_native.Height = res_desc.height;
        res_desc_native.Layout = (D3D12_TEXTURE_LAYOUT)res_desc.texture_layout;
        res_desc_native.MipLevels = res_desc.mip_levels;
        res_desc_native.SampleDesc.Count = res_desc.sample_desc.count;
        res_desc_native.SampleDesc.Quality = res_desc.sample_desc.quality;
        res_desc_native.Width = res_desc.width;

        if (clear_val) {
            clear_val_native.Format = (DXGI_FORMAT)clear_val->format;
            if (clear_val->isDepth) {
                clear_val_native.DepthStencil.Depth = clear_val->depth_tencil.depth;
                clear_val_native.DepthStencil.Stencil = clear_val->depth_tencil.stencil;
            }
            else {
                memcpy(clear_val_native.Color, clear_val->color, sizeof(float) * 4);
            }
        }
    }

    ThrowIfFailed(gBackend->GetDevice()->GetNativeObject()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(internal_type),
        D3D12_HEAP_FLAG_NONE,
        &res_desc_native,
        (D3D12_RESOURCE_STATES)initial_state,
        (clear_val ? &clear_val_native : nullptr),
        IID_PPV_ARGS(&m_resourse)
    ));
    SetName(m_resourse, dbg_name.value_or(L"").append(L"_texture").c_str());
    
    m_recreate_intermediate_res = true;
}

void HeapBuffer::Load(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData){
    if (bufferData)
    {
        auto device = gBackend->GetDevice();
        const size_t bufferSize = numElements * elementSize;
        if (m_recreate_intermediate_res)
        {
            pIntermediateResource.Reset();
            ThrowIfFailed(device->GetNativeObject()->CreateCommittedResource(
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

        UpdateSubresources(command_list->GetRawCommandList().Get(),
            m_resourse.Get(), pIntermediateResource.Get(),
            0, 0, 1, &subresourceData);
    }
}

void HeapBuffer::Load(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData){
    if (subresourceData)
    {
        auto device = gBackend->GetDevice();
        const uint64_t required_size = GetRequiredIntermediateSize(m_resourse.Get(), firstSubresource, numSubresources);
        if (m_recreate_intermediate_res)
        {
            pIntermediateResource.Reset();
            ThrowIfFailed(device->GetNativeObject()->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(required_size),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(pIntermediateResource.GetAddressOf())));
                SetName(pIntermediateResource, L"_intermediate");
            
            m_recreate_intermediate_res = !m_recreate_intermediate_res;
        }

        UpdateSubresources(command_list->GetRawCommandList().Get(),
            m_resourse.Get(), pIntermediateResource.Get(),
            0, firstSubresource, numSubresources, (D3D12_SUBRESOURCE_DATA*)subresourceData);
    }
}

uint8_t* HeapBuffer::Map(){
    BYTE* pData;
    ThrowIfFailed(m_resourse->Map(0, nullptr, reinterpret_cast<void**>(&pData)));
    m_cpu_data = pData;

    return (uint8_t*)pData;
}

void HeapBuffer::Unmap(){
    m_cpu_data = nullptr;
    m_resourse->Unmap(0, nullptr);
}

//void HeapBuffer::Copy(const CommandList &command_list, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size){
//    command_list->CopyBufferRegion(dest.m_resourse.Get(), dstOffset, m_resourse.Get(), srcOffset, size);
//}