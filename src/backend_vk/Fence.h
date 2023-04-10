#pragma one

#include "IFence.h"
#include <volk.h>

class Fence : public IFence {
public:
    void Initialize(uint32_t val) override;
    VkFence &GetFence() {
        return m_fence;
    }
    VkSemaphore &GetSemaphore() {
        return m_semaphore;
    }
private:
    VkFence m_fence;
    VkSemaphore m_semaphore;
};