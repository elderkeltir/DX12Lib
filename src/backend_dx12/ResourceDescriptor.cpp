#include "ResourceDescriptor.h"
#include <directx/d3dx12.h>
#include "HeapBuffer.h"
#include "DxBackend.h"
#include "DescriptorHeapCollection.h"
#include "dx12_helper.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

#define GetDxHeap(heap) ((HeapBuffer*)heap.get())

bool ResourceDescriptor::Create_RTV(std::weak_ptr<IHeapBuffer> buff){
    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveRTVhandle(m_cpu_handle);
        if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
            D3D12_CPU_DESCRIPTOR_HANDLE hndl;
            hndl.ptr = m_cpu_handle.ptr;
            gBackend->GetDevice()->GetNativeObject()->CreateRenderTargetView(GetDxHeap(buffer)->GetResource().Get(), nullptr, hndl);
            m_type = ResourceDescriptorType::rdt_rtv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_DSV(std::weak_ptr<IHeapBuffer> buff, const DSVdesc &desc){
    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveDSVhandle(m_cpu_handle);
        if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
            D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
            dsv_desc.Format = (DXGI_FORMAT)desc.format;
            dsv_desc.ViewDimension = (D3D12_DSV_DIMENSION)desc.dimension;
            dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
            dsv_desc.Texture2D.MipSlice = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE hndl;
            hndl.ptr = m_cpu_handle.ptr;
            gBackend->GetDevice()->GetNativeObject()->CreateDepthStencilView(GetDxHeap(buffer)->GetResource().Get(), &dsv_desc, hndl);
            m_type = ResourceDescriptorType::rdt_dsv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_SRV(std::weak_ptr<IHeapBuffer> buff, const SRVdesc &desc){
    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Format = (DXGI_FORMAT)desc.format;
            srv_desc.ViewDimension = (D3D12_SRV_DIMENSION)desc.dimension; // TODO: same as uav and rest?...
            if (desc.dimension == SRVdesc::SRVdimensionType::srv_dt_texture2d) {
                srv_desc.Texture2D.MostDetailedMip = desc.texture2d.most_detailed_mip;
                srv_desc.Texture2D.MipLevels = desc.texture2d.mip_levels;
                srv_desc.Texture2D.ResourceMinLODClamp = desc.texture2d.res_min_lod_clamp;
                srv_desc.Texture2D.PlaneSlice = 0;
            }
            else if (desc.dimension == SRVdesc::SRVdimensionType::srv_dt_buffer) {
                srv_desc.Buffer.FirstElement = desc.buffer.first_element;
                srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                srv_desc.Buffer.NumElements = desc.buffer.num_elements;
                srv_desc.Buffer.StructureByteStride = desc.buffer.structure_byte_stride;
            }
            else if (desc.dimension == SRVdesc::SRVdimensionType::srv_dt_texturecube) {
                srv_desc.TextureCube.MostDetailedMip = desc.texture_cube.most_detailed_mip;
                srv_desc.TextureCube.MipLevels = desc.texture_cube.mip_levels;
                srv_desc.TextureCube.ResourceMinLODClamp = desc.texture_cube.res_min_lod_clamp;
            }

            D3D12_CPU_DESCRIPTOR_HANDLE hndl;
            hndl.ptr = m_cpu_handle.ptr;
            gBackend->GetDevice()->GetNativeObject()->CreateShaderResourceView(GetDxHeap(buffer)->GetResource().Get(), &srv_desc, hndl);
            m_type = ResourceDescriptorType::rdt_srv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_UAV(std::weak_ptr<IHeapBuffer> buff, const UAVdesc &desc){
    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
            D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
            uav_desc.Format = (DXGI_FORMAT)desc.format;
            uav_desc.ViewDimension = (D3D12_UAV_DIMENSION)desc.dimension;
            uav_desc.Texture2D.MipSlice = desc.texture2d.mip_slice;
            uav_desc.Texture2D.PlaneSlice = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE hndl;
            hndl.ptr = m_cpu_handle.ptr;
            gBackend->GetDevice()->GetNativeObject()->CreateUnorderedAccessView(GetDxHeap(buffer)->GetResource().Get(), nullptr, &uav_desc, hndl);
            m_type = ResourceDescriptorType::rdt_uav;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_CBV(std::weak_ptr<IHeapBuffer> buff, const CBVdesc &desc) {
    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            cbv_desc.SizeInBytes = desc.size_in_bytes;
            cbv_desc.BufferLocation = GetDxHeap(buffer)->GetResource()->GetGPUVirtualAddress();
            D3D12_CPU_DESCRIPTOR_HANDLE hndl;
            hndl.ptr = m_cpu_handle.ptr;
            gBackend->GetDevice()->GetNativeObject()->CreateConstantBufferView(&cbv_desc, hndl);
            m_type = ResourceDescriptorType::rdt_cbv;

            return true;
        }
    }
    return false;
}