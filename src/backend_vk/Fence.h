#pragma once

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
    bool WasSignaled() const {
        return  m_signaled;
    }
    void SetSignaled(bool sig) {
        m_signaled = sig;
    }
private:
    VkFence m_fence;
    VkSemaphore m_semaphore;
    bool m_signaled {false};
};