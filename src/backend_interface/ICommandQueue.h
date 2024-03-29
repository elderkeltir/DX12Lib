#pragma once

#include <string>
#include <optional>
#include <memory>

class IFence;
class ICommandList;
class IDynamicGpuHeap;

class ICommandQueue {
public:
    enum class QueueType {
        qt_gfx,
        qt_compute,
        qt_copy
    };

    virtual void OnInit(QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void OnDestroy() = 0;

    virtual uint32_t Signal() = 0;
    virtual void Signal(std::unique_ptr<IFence>& fence, uint32_t fence_value) = 0;
    virtual void WaitOnCPU(uint32_t fence_value) = 0;
    virtual void WaitOnGPU(std::unique_ptr<IFence>& fence, uint32_t fence_value) = 0;
    virtual void Flush() = 0;

    virtual ICommandList* ResetActiveCL() = 0;
    virtual ICommandList* GetActiveCL() = 0;
    virtual void ExecuteActiveCL() = 0;

    virtual IDynamicGpuHeap& GetGpuHeap() = 0;
    virtual ~ICommandQueue() = default;
};