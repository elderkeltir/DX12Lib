#pragma once

#include "ICommandQueue.h"
#include <vector>
#include <memory>
#include <array>
#include <volk.h>

class IFence;

class CommandQueue : public ICommandQueue {
public:
    struct SubmitUnit {
        
        
    };
    void OnInit(QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void OnDestroy() override;

    // not used
    void Signal(std::unique_ptr<IFence>& fence, uint32_t fence_value) override {}
    void WaitOnGPU(std::unique_ptr<IFence>& fence, uint32_t fence_value) override {}
    //
    uint32_t Signal() override;
    void WaitOnCPU(uint32_t fence_value) override;
    void Signal(IFence* fence, bool on_cpu) override;
    void WaitOnCPU(IFence* fence) override;
    void WaitOnGPU(IFence* fence) override;

    void Flush() override;

    ICommandList* ResetActiveCL() override;
    ICommandList* GetActiveCL() override;
    void ExecuteActiveCL() override;

    IDynamicGpuHeap& GetGpuHeap() override;

    VkQueue GetNativeObject() {
        return m_queue;
    }

private:
    uint32_t TestFamilQueueyIndex(uint8_t queueFlags);
    
    uint32_t m_active_cl{0};
    uint32_t m_command_list_num{ 0 };
    QueueType m_type;
    std::unique_ptr<IFence> m_fence;
    VkCommandBuffer m_cmd_list_cpu_sync;
    VkCommandBuffer m_cmd_list_gpu_sync;

    VkQueue m_queue;
    VkCommandPool m_command_pool;
    std::array<std::unique_ptr<IDynamicGpuHeap>, 2> m_dynamic_gpu_heaps;

    std::vector<ICommandList*> cmd_lists;
};
