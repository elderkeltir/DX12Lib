#include "ResourceDescriptor.h"
#include "HeapBuffer.h"
#include "VkBackend.h"

extern VkBackend* gBackend;

const ImageMemAllocation& GetImageData(std::weak_ptr<IHeapBuffer> buff) {
    if (std::shared_ptr<IHeapBuffer> buffer = buff.lock()){
        HeapBuffer* buff_native = (HeapBuffer*)buffer.get();
        assert(buff_native->GetVkType() == HeapBuffer::BufferResourceType::rt_texture);
        
        return buff_native->GetImageInfo();
    }
}

bool ResourceDescriptor::Create_RTV(std::weak_ptr<IHeapBuffer> buff) {
    const ImageMemAllocation& img_data = GetImageData(buff);

    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveRTVhandle(m_cpu_handle, (uint64_t)&img_data);
    }
}

bool ResourceDescriptor::Create_DSV(std::weak_ptr<IHeapBuffer> buff, const DSVdesc& desc) {
    const ImageMemAllocation& img_data = GetImageData(buff);

    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveDSVhandle(m_cpu_handle, (uint64_t)&img_data);
    }
}

bool ResourceDescriptor::Create_SRV(std::weak_ptr<IHeapBuffer> buff, const SRVdesc& desc) {
    const ImageMemAllocation& img_data = GetImageData(buff);

    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveSRVhandle(m_cpu_handle, (uint64_t)&img_data);
    }
}

bool ResourceDescriptor::Create_UAV(std::weak_ptr<IHeapBuffer> buff, const UAVdesc& desc) {
    const ImageMemAllocation& img_data = GetImageData(buff);

    if (IDescriptorHeapCollection* descriptorHeapCollection = gBackend->GetDescriptorHeapCollection()){
        descriptorHeapCollection->ReserveUAVhandle(m_cpu_handle, (uint64_t)&img_data);
    }
}

bool ResourceDescriptor::Create_CBV(std::weak_ptr<IHeapBuffer> buff, const CBVdesc& desc) {
    return true; // TODO: no need for view
}

