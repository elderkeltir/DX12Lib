#pragma once
#include "DXApp.h"
#include "ResourceManager.h"
#include "ConstantBufferManager.h"
#include "Techniques.h"
#include "Logger.h"

#include <optional>
#include <chrono>

using Microsoft::WRL::ComPtr;
class Level;
class DescriptorHeapCollection;
class GpuResource;
class GfxCommandQueue;
class CommandList;
class FreeCamera;
class RenderQuad;
class DynamicGpuHeap;
class SSAO;
class ImguiHelper;
class Reflections;


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
    uint32_t FrameNumber() const { return m_frame_id; }
    uint32_t FrameId() const { return m_frameIndex; }
    float GetAspectRatio() const { return m_width/(float)m_height; }
    std::weak_ptr<GfxCommandQueue> GetGfxQueue() { return m_commandQueueGfx; }
    std::weak_ptr<GfxCommandQueue> GetComputeQueue() { return m_commandQueueCompute; }

    // Events
    virtual void OnMouseMoved(WPARAM btnState, int x, int y) override;
    virtual void OnKeyDown(UINT8 key) override;
    virtual void OnKeyUp(UINT8 key) override;
    virtual void RebuildShaders(std::optional<std::wstring> dbg_name = std::nullopt) override;
    logger* GetLogger() { return m_logger.get(); }
    ImguiHelper* GetUiHelper() { return m_gui.get(); }
    uint32_t GetRenderMode() const { return m_render_mode; }
    void SetRenderMode(uint32_t mode) { m_render_mode = mode; }
private:
    static constexpr uint32_t FrameCount = 2;
    static constexpr uint32_t GfxQueueCmdList_num = 6;
    static constexpr uint32_t ComputeQueueCmdList_num = 6;
    
    void CreateDevice(std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateSwapChain(std::optional<std::wstring> dbg_name = std::nullopt);
    void PrepareRenderTarget(CommandList& command_list, const std::vector<std::shared_ptr<GpuResource>> &rt, bool set_dsv = true, bool clear_dsv = true);
    void PrepareRenderTarget(CommandList& command_list, GpuResource &rts, bool set_dsv = true, bool clear_dsv = true);
    void RenderLevel(CommandList& command_list);
    void RenderPostProcessQuad(CommandList& command_list);
    void RenderForwardQuad(CommandList& command_list);
    void RenderDeferredShadingQuad(CommandList& command_list);
    void RenderSSAOquad(CommandList& command_list);
    void BlurSSAO(CommandList& command_list);
    void GenerateReflections(CommandList& command_list);

    void UpdateCamera(std::shared_ptr<FreeCamera> &camera, float dt);

    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    ComPtr<IDXGIFactory4> m_factory;
    ComPtr<ID3D12Device2> m_device;
    ComPtr<IDXGISwapChain4> m_swapChain;

    ComPtr<ID3D12Fence> m_fence_inter_queue;
    uint64_t m_fence_inter_queue_val{ 0 };
    
    std::shared_ptr<GfxCommandQueue> m_commandQueueGfx;
    std::shared_ptr<GfxCommandQueue> m_commandQueueCompute;
    std::unique_ptr<GpuResource[]> m_renderTargets;
    std::unique_ptr<GpuResource> m_depthStencil;
    std::unique_ptr<RenderQuad> m_post_process_quad;
    std::unique_ptr<RenderQuad> m_deferred_shading_quad;
    std::unique_ptr<RenderQuad> m_forward_quad;
    std::unique_ptr<SSAO> m_ssao;
    std::unique_ptr<Reflections> m_reflections;
    std::unique_ptr<ImguiHelper> m_gui;
    std::unique_ptr<logger> m_logger;

    uint32_t m_frameIndex;
    uint64_t m_fenceValues[FrameCount]{0};
    
    std::chrono::time_point<std::chrono::system_clock> m_time;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::duration<float> m_total_time;
    std::chrono::duration<float> m_dt;
    uint32_t m_frame_id{0};

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
    uint32_t m_render_mode{ 0 };
    bool m_rebuild_shaders{ false };
};

#define LOG_CRITICAL(msg, ...) gD3DApp->GetLogger()->hlog(logger::log_level::ll_CRITICAL, msg, __VA_ARGS__)
#define LOG_ERROR(msg, ...) gD3DApp->GetLogger()->hlog(logger::log_level::ll_ERROR, msg, __VA_ARGS__)
#define LOG_WARNING(msg, ...) gD3DApp->GetLogger()->hlog(logger::log_level::ll_WARNING, msg, __VA_ARGS__)
#define LOG_DEBUG(msg, ...) gD3DApp->GetLogger()->hlog(logger::log_level::ll_DEBUG, msg, __VA_ARGS__)
#define LOG_INFO(msg, ...) gD3DApp->GetLogger()->hlog(logger::log_level::ll_INFO, msg, __VA_ARGS__)
