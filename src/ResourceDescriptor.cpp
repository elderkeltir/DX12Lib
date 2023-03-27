#include "ResourceDescriptor.h"

#include "HeapBuffer.h"
#include "DXAppImplementation.h"
#include "DescriptorHeapCollection.h"
#include "DXHelper.h"

extern DXAppImplementation *gD3DApp;

bool ResourceDescriptor::Create_RTV(std::weak_ptr<HeapBuffer> buff){
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveRTVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            gD3DApp->GetDevice()->CreateRenderTargetView(buffer->GetResource().Get(), nullptr, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_rtv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_DSV(std::weak_ptr<HeapBuffer> buff, const DSVdesc &desc){
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveDSVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
            dsv_desc.Format = (DXGI_FORMAT)desc.format;
            dsv_desc.ViewDimension = (D3D12_DSV_DIMENSION)desc.dimension;
            dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
            dsv_desc.Texture2D.MipSlice = 0;
            gD3DApp->GetDevice()->CreateDepthStencilView(buffer->GetResource().Get(), &dsv_desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_dsv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_SRV(std::weak_ptr<HeapBuffer> buff, const SRVdesc &desc){
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
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
            
            gD3DApp->GetDevice()->CreateShaderResourceView(buffer->GetResource().Get(), &srv_desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_srv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_UAV(std::weak_ptr<HeapBuffer> buff, const UAVdesc &desc){
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
            uav_desc.Format = (DXGI_FORMAT)desc.format;
            uav_desc.ViewDimension = (D3D12_UAV_DIMENSION)desc.dimension;
            uav_desc.Texture2D.MipSlice = desc.texture2d.mip_slice;
            uav_desc.Texture2D.PlaneSlice = 0;
            gD3DApp->GetDevice()->CreateUnorderedAccessView(buffer->GetResource().Get(), nullptr, &uav_desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_uav;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_CBV(std::weak_ptr<HeapBuffer> buff, const CBVdesc &desc) {
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveSRVUAVCBVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            cbv_desc.SizeInBytes = desc.size_in_bytes;
            cbv_desc.BufferLocation = buffer->GetResource()->GetGPUVirtualAddress();
            gD3DApp->GetDevice()->CreateConstantBufferView(&cbv_desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_cbv;

            return true;
        }
    }
    return false;
}