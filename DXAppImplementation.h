#pragma once
#include "DXApp.h"
#include "ResourceManager.h"

using Microsoft::WRL::ComPtr;
class Level;

class DXAppImplementation : public DXApp, public ResourceManager
{
public:
    DXAppImplementation(uint32_t width, uint32_t height, std::wstring name);
    ~DXAppImplementation();

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;

    Level* GetLevel();

private:
    static constexpr uint32_t FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Device2> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    uint32_t m_rtvDescriptorSize;

    // Synchronization objects.
    uint32_t m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValue{0};
    uint64_t m_fenceValues[FrameCount]{0};

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    uint64_t Signal(uint64_t &fenceValue);
    void Wait(uint64_t fenceValue);

    //
    std::unique_ptr<Level> m_level;
};