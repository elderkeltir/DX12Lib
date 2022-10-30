#include "CommandQueue.h"
#include "DXHelper.h"

void CommandQueue::Flush()
{
    const uint64_t fence_value = Signal();
    Wait(fence_value);
}

uint64_t CommandQueue::Signal(){
    uint64_t fenceValueForSignal = ++m_fence_value;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void CommandQueue::Wait(uint64_t fence_value){
    if (m_fence->GetCompletedValue() < fence_value) {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence_value, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}