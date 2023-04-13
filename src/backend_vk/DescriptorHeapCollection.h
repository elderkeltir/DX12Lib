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
    VkDescriptorPool GetPool(uint32_t frame_id, uint32_t queue_id) { // TODO: queue_id=(0=gfx, 1=compute), frame={0,1}
        uint32_t id = frame_id * 2 + queue_id;
        return m_pool_cpu[id];
    }
    ~DescriptorHeapCollection() override;
private:
    const uint32_t srv_img_desc_count = 64;
    const uint32_t uav_img_desc_count = 64;
    const uint32_t srv_buf_desc_count = 64;
    const uint32_t cbv_buf_desc_count = 64;
    const uint32_t uav_buf_desc_count = 64;
    static const uint32_t frame_num = 2; // TODO: make this 2 appears only once in the code pls <3
    VkDescriptorPool m_pool_gpu[frame_num * 2]; // TODO: 2x for each queue
    VkDescriptorPool m_pool_cpu[frame_num * 2];
};