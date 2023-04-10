#pragma once

#include "IDescriptorHeapCollection.h"
#include <volk.h>

class DescriptorHeapCollection : public IDescriptorHeapCollection {
public:
    void Initialize(std::optional<std::wstring> dbg_name = std::nullopt) override;
    void ReserveRTVhandle(CPUdescriptor& rtvHandle, uint64_t data = 0, bool gpu_only = false) override;
    void ReserveDSVhandle(CPUdescriptor& dsvHandle, uint64_t data = 0, bool gpu_only = false) override;
    void ReserveCBVhandle(CPUdescriptor& cbvHandle, uint64_t data = 0, bool gpu_only = false) override;
    void ReserveSRVhandle(CPUdescriptor& srvHandle, uint64_t data = 0, bool gpu_only = false) override;
    void ReserveUAVhandle(CPUdescriptor& uavHandle, uint64_t data = 0, bool gpu_only = false) override;
    ~DescriptorHeapCollection() override;
private:
    const uint32_t srv_img_desc_count = 64;
    const uint32_t uav_img_desc_count = 64;
    const uint32_t srv_buf_desc_count = 64;
    const uint32_t cbv_buf_desc_count = 64;
    const uint32_t uav_buf_desc_count = 64;
    VkDescriptorPool m_pool_gpu;
    VkDescriptorPool m_pool_cpu;
};