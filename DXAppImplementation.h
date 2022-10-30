#pragma once
#include "DXApp.h"
#include "ResourceManager.h"
#include "ConstantBufferManager.h"

#include <chrono>

using Microsoft::WRL::ComPtr;
class Level;
class DescriptorHeapCollection;
class GpuResource;
class GfxCommandQueue;

class DXAppImplementation : public DXApp, public ResourceManager, public ConstantBufferManager
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

    const std::chrono::duration<float>& TotalTime() const { return m_total_time; }
    const std::chrono::duration<float>& FrameTime() const { return m_dt; }
    uint64_t FrameNumber() const { return m_frame_id; }
    float GetAspectRatio() const { return m_width/(float)m_height; }

private:
    static constexpr uint32_t FrameCount = 2;
    
    void CreateDevice();
    void CreateSwapChain();
    void PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6> &command_list);
    
    ComPtr<IDXGIFactory4> m_factory;
    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Device2> m_device;
    std::shared_ptr<GfxCommandQueue> m_commandQueueGfx;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState; // empty? remove later


    // Pipeline state object.
    ComPtr<ID3D12PipelineState> m_PipelineState;

    std::unique_ptr<GpuResource[]> m_renderTargets;
    std::unique_ptr<GpuResource> m_depthStencil;

    uint32_t m_frameIndex;
    uint64_t m_fenceValues[FrameCount]{0};
    
    //
    void LoadTechnique();
    void RenderCube(ComPtr<ID3D12GraphicsCommandList6>& command_list);
    std::chrono::time_point<std::chrono::system_clock> m_time;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::duration<float> m_total_time;
    std::chrono::duration<float> m_dt;
    uint64_t m_frame_id{0};

    //
    std::shared_ptr<Level> m_level;
    std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
};