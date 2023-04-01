#pragma once

#include <string>
#include <memory>
#include <chrono>

#include "ResourceManager.h"
#include "ConstantBufferManager.h"
#include "ITechniques.h"

struct WindowHandler;
class IBackend;
class RenderQuad;
class SSAO;
class Reflections;
class Level;
class FreeCamera;
class IGpuResource;
class ICommandList;
class IRootSignature;
class ICommandQueue;


class Frontend : public ResourceManager, public ConstantBufferManager
{
public:
    enum class MouseButton {
        m_undefined,
        mb_right,
        mb_left,
        mb_middle
    };
    enum class KeyboardButton {
        kb_none,
        kb_w,
        kb_a,
        kb_s,
        kb_d,
        kb_tilda
    };
public:
    Frontend(uint32_t width, uint32_t height, std::wstring name);
    ~Frontend();

    void OnInit(const WindowHandler &hwnd);
    void OnUpdate();
    void OnRender();
    void OnDestroy();

    // Samples override the event handlers to handle specific messages.
    void OnKeyDown(KeyboardButton key);
    void OnKeyUp(KeyboardButton key);
    void OnMouseButtonDown(int x, int y) {}
    void OnMouseMoved(MouseButton bnt, int x, int y);

    // Accessors.
    uint32_t GetWidth() const       { return m_width; }
    uint32_t GetHeight() const      { return m_height; }
    const wchar_t* GetTitle() const   { return m_title.c_str(); }
    float GetAspectRatio() const { return m_width / (float)m_height; }
    const std::chrono::duration<float>& TotalTime() const { return m_total_time; }
    const std::chrono::duration<float>& FrameTime() const { return m_dt; }
    uint32_t FrameNumber() const { return m_frame_id; }
    uint32_t FrameId() const;
    std::weak_ptr<Level> GetLevel() { return m_level; }
    uint32_t GetRenderMode() const;

    const ITechniques::Technique* GetTechniqueById(uint32_t id) const;
    const IRootSignature* GetRootSignById(uint32_t id);

    ICommandQueue* GetGfxQueue();
    ICommandQueue* GetComputeQueue();
    void SetRenderMode(uint32_t mode);
    void RebuildShaders();

    bool ShouldClose() const { return m_should_close; }
    void Close() { m_should_close = true; }
protected:
    void UpdateCamera(std::shared_ptr<FreeCamera>& camera, float dt);
    void PrepareRenderTarget(ICommandList* command_list, const std::vector<std::shared_ptr<IGpuResource>>& rt, bool set_dsv = true, bool clear_dsv = true);
    void PrepareRenderTarget(ICommandList* command_list, IGpuResource& rts, bool set_dsv = true, bool clear_dsv = true);
    void RenderLevel(ICommandList* command_list);
    void RenderPostProcessQuad(ICommandList* command_list);
    void RenderForwardQuad(ICommandList* command_list);
    void RenderDeferredShadingQuad(ICommandList* command_list);
    void RenderSSAOquad(ICommandList* command_list);
    void BlurSSAO(ICommandList* command_list);
    void GenerateReflections(ICommandList* command_list);

    IGpuResource& GetCurrentRT();
    IGpuResource* GetDepthBuffer();

    std::unique_ptr<IBackend> m_backend;
    // Viewport dimensions.
    uint32_t m_width;
    uint32_t m_height;
    float m_aspectRatio;

    std::unique_ptr<RenderQuad> m_post_process_quad;
    std::unique_ptr<RenderQuad> m_deferred_shading_quad;
    std::unique_ptr<RenderQuad> m_forward_quad;
    std::unique_ptr<SSAO> m_ssao;
    std::unique_ptr<Reflections> m_reflections;
    std::unique_ptr<ITechniques> m_techniques;

    std::chrono::time_point<std::chrono::system_clock> m_time;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::duration<float> m_total_time;
    std::chrono::duration<float> m_dt;
    uint32_t m_frame_id{ 0 };

    std::shared_ptr<Level> m_level;

    struct {
        enum camera_movement_type {
            cm_fwd = 1 << 0,
            cm_bcwd = 1 << 1,
            cm_right = 1 << 2,
            cm_left = 1 << 3
        };
        uint8_t camera_movement_state{ 0 };
        uint32_t camera_x{ 0 };
        uint32_t camera_y{ 0 };
        int32_t camera_x_delta{ 0 };
        int32_t camera_y_delta{ 0 };
    } m_camera_movement;

    bool m_should_close{ false };

private:
    // Window title.
    std::wstring m_title;
};