#pragma once

#include <volk.h>
#include <vector>

class VkDeviceW {
public:
    void Init(VkInstance instance);
    uint32_t TestFamilQueueyIndex(VkPhysicalDevice phys_device, uint8_t queueFlags) const;
    VkDevice GetNativeObject() { 
        return m_device;
    }

    VkPhysicalDevice GetPhysicalDevice() {
        return m_phys_device;
    }

    uint32_t GetFamilyIndex(bool gfx) {
        return (gfx ? m_family_idx_gfx : m_family_idx_compute);
    }

private:
    VkPhysicalDevice PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices);
    void CreateDevice();
    
    VkDevice m_device;
    VkPhysicalDevice m_phys_device;

    uint32_t m_family_idx_gfx;
    uint32_t m_family_idx_compute;
};
