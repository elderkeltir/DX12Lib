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

bool ResourceDescriptor::Create_DSV(std::weak_ptr<HeapBuffer> buff, const D3D12_DEPTH_STENCIL_VIEW_DESC &desc){
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveDSVhandle(m_cpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            gD3DApp->GetDevice()->CreateDepthStencilView(buffer->GetResource().Get(), &desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_dsv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_SRV(std::weak_ptr<HeapBuffer> buff, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool gpu_visible){
    assert(gpu_visible);
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveSRVUAVhandle(m_cpu_handle, m_gpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            gD3DApp->GetDevice()->CreateShaderResourceView(buffer->GetResource().Get(), &desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_srv;

            return true;
        }
    }
    return false;
}

bool ResourceDescriptor::Create_UAV(std::weak_ptr<HeapBuffer> buff, const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc, bool gpu_visible){
    assert(gpu_visible);
    if (std::shared_ptr<DescriptorHeapCollection> descriptorHeapCollection = gD3DApp->GetDescriptorHeapCollection().lock()){
        descriptorHeapCollection->ReserveSRVUAVhandle(m_cpu_handle, m_gpu_handle);
        if (std::shared_ptr<HeapBuffer> buffer = buff.lock()){
            gD3DApp->GetDevice()->CreateUnorderedAccessView(buffer->GetResource().Get(), nullptr, &desc, m_cpu_handle);
            m_type = ResourceDescriptorType::rdt_uav;

            return true;
        }
    }
    return false;
}