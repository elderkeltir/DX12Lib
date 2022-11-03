#pragma once
#include "DXApp.h"
#include "ResourceManager.h"
#include "ConstantBufferManager.h"
#include "Techniques.h"

#include <optional>
#include <chrono>

using Microsoft::WRL::ComPtr;
class Level;
class DescriptorHeapCollection;
class GpuResource;
class GfxCommandQueue;
class FreeCamera;

class DXAppImplementation : public DXApp, public ResourceManager, public ConstantBufferManager, public Techniques
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

    // Events
    virtual void OnMouseMoved(WPARAM btnState, int x, int y) override;
    virtual void OnKeyDown(UINT8 key) override;
    virtual void OnKeyUp(UINT8 key) override;
private:
    static constexpr uint32_t FrameCount = 2;
    
    void CreateDevice(std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateSwapChain(std::optional<std::wstring> dbg_name = std::nullopt);
    void PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6> &command_list);
    void RenderLevel(ComPtr<ID3D12GraphicsCommandList6>& command_list);

    void UpdateCamera(std::shared_ptr<FreeCamera> &camera, float dt);

    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    ComPtr<IDXGIFactory4> m_factory;
    ComPtr<ID3D12Device2> m_device;
    ComPtr<IDXGISwapChain4> m_swapChain;
    
    std::unique_ptr<GfxCommandQueue> m_commandQueueGfx;
    std::unique_ptr<GpuResource[]> m_renderTargets;
    std::unique_ptr<GpuResource> m_depthStencil;

    uint32_t m_frameIndex;
    uint64_t m_fenceValues[FrameCount]{0};
    
    std::chrono::time_point<std::chrono::system_clock> m_time;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::duration<float> m_total_time;
    std::chrono::duration<float> m_dt;
    uint64_t m_frame_id{0};

    std::shared_ptr<Level> m_level;
    std::shared_ptr<DescriptorHeapCollection> m_descriptor_heap_collection;
    struct {
        enum camera_movement_type {
            cm_fwd      = 1 << 0,
            cm_bcwd     = 1 << 1,
            cm_right    = 1 << 2,
            cm_left     = 1 << 3
        };
        uint8_t camera_movement_state{0};
        uint32_t camera_x{0};
        uint32_t camera_y{0};
        int32_t camera_x_delta{0};
        int32_t camera_y_delta{0};
    } m_camera_movement;
};