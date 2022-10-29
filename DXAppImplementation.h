#pragma once
#include "DXApp.h"
#include "ResourceManager.h"

#include <chrono>

using Microsoft::WRL::ComPtr;
class Level;
class DescriptorHeapCollection;
class GpuResource;

class DXAppImplementation : public DXApp, public ResourceManager
{
public:
    DXAppImplementation(uint32_t width, uint32_t height, std::wstring name);
    ~DXAppImplementation();

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;

    std::weak_ptr<Level> GetLevel() { return m_level; }
    std::weak_ptr<DescriptorHeapCollection> GetDescriptorHeapCollection() {return m_descriptor_heap_collection; }

    ComPtr<ID3D12Device2>& GetDevice() { return m_device; }
private:
    static constexpr uint32_t FrameCount = 2;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Device2> m_device;
    //ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    //ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    //ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    //ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;
    //uint32_t m_rtvDescriptorSize;
    // Pipeline state object.
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    // Vertex buffer for the cube.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    // Index buffer for the cube.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    std::unique_ptr<GpuResource[]> m_renderTargets;
    std::unique_ptr<GpuResource> m_depthStencil;

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
    void LoadTechnique();
    void RenderCube();
    std::chrono::time_point<std::chrono::system_clock> m_time;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::duration<double> m_total_time;
    std::chrono::duration<double> m_dt;

    //
    std::shared_ptr<Level> m_level;
    std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
};