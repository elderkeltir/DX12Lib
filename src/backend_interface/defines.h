#pragma once

#include <cstdint>
#include <vector>

enum ResourceState {
    rs_resource_state_common = 0,
    rs_resource_state_vertex_and_constant_buffer = 0x1,
    rs_resource_state_index_buffer = 0x2,
    rs_resource_state_render_target = 0x4,
    rs_resource_state_unordered_access = 0x8,
    rs_resource_state_depth_write = 0x10,
    rs_resource_state_depth_read = 0x20,
    rs_resource_state_non_pixel_shader_resource = 0x40,
    rs_resource_state_pixel_shader_resource = 0x80,
    rs_resource_state_stream_out = 0x100,
    rs_resource_state_indirect_argument = 0x200,
    rs_resource_state_copy_dest = 0x400,
    rs_resource_state_copy_source = 0x800,
    rs_resource_state_resolve_dest = 0x1000,
    rs_resource_state_resolve_source = 0x2000,
    rs_resource_state_raytracing_acceleration_structure = 0x400000,
    rs_resource_state_shading_rate_source = 0x1000000,
    rs_resource_state_generic_read = (((((rs_resource_state_vertex_and_constant_buffer | rs_resource_state_index_buffer) | rs_resource_state_non_pixel_shader_resource) | rs_resource_state_pixel_shader_resource) | rs_resource_state_indirect_argument) | rs_resource_state_copy_source),
    rs_resource_state_all_shader_resource = (rs_resource_state_non_pixel_shader_resource | rs_resource_state_pixel_shader_resource),
    rs_resource_state_present = 0,
    rs_resource_state_predication = rs_resource_state_indirect_argument,
    rs_resource_state_video_decode_read = 0x10000,
    rs_resource_state_video_decode_write = 0x20000,
    rs_resource_state_video_process_read = 0x40000,
    rs_resource_state_video_process_write = 0x80000,
    rs_resource_state_video_encode_read = 0x200000,
    rs_resource_state_video_encode_write = 0x800000
};

enum class ResourceFormat : uint64_t {
    rf_unknown = 0,
    rf_r32g32b32a32_typeless = 1,
    rf_r32g32b32a32_float = 2,
    rf_r32g32b32a32_uint = 3,
    rf_r32g32b32a32_sint = 4,
    rf_r32g32b32_typeless = 5,
    rf_r32g32b32_float = 6,
    rf_r32g32b32_uint = 7,
    rf_r32g32b32_sint = 8,
    rf_r16g16b16a16_typeless = 9,
    rf_r16g16b16a16_float = 10,
    rf_r16g16b16a16_unorm = 11,
    rf_r16g16b16a16_uint = 12,
    rf_r16g16b16a16_snorm = 13,
    rf_r16g16b16a16_sint = 14,
    rf_r32g32_typeless = 15,
    rf_r32g32_float = 16,
    rf_r32g32_uint = 17,
    rf_r32g32_sint = 18,
    rf_r32g8x24_typeless = 19,
    rf_d32_float_s8x24_uint = 20,
    rf_r32_float_x8x24_typeless = 21,
    rf_x32_typeless_g8x24_uint = 22,
    rf_r10g10b10a2_typeless = 23,
    rf_r10g10b10a2_unorm = 24,
    rf_r10g10b10a2_uint = 25,
    rf_r11g11b10_float = 26,
    rf_r8g8b8a8_typeless = 27,
    rf_r8g8b8a8_unorm = 28,
    rf_r8g8b8a8_unorm_srgb = 29,
    rf_r8g8b8a8_uint = 30,
    rf_r8g8b8a8_snorm = 31,
    rf_r8g8b8a8_sint = 32,
    rf_r16g16_typeless = 33,
    rf_r16g16_float = 34,
    rf_r16g16_unorm = 35,
    rf_r16g16_uint = 36,
    rf_r16g16_snorm = 37,
    rf_r16g16_sint = 38,
    rf_r32_typeless = 39,
    rf_d32_float = 40,
    rf_r32_float = 41,
    rf_r32_uint = 42,
    rf_r32_sint = 43,
    rf_r24g8_typeless = 44,
    rf_d24_unorm_s8_uint = 45,
    rf_r24_unorm_x8_typeless = 46,
    rf_x24_typeless_g8_uint = 47,
    rf_r8g8_typeless = 48,
    rf_r8g8_unorm = 49,
    rf_r8g8_uint = 50,
    rf_r8g8_snorm = 51,
    rf_r8g8_sint = 52,
    rf_r16_typeless = 53,
    rf_r16_float = 54,
    rf_d16_unorm = 55,
    rf_r16_unorm = 56,
    rf_r16_uint = 57,
    rf_r16_snorm = 58,
    rf_r16_sint = 59,
    rf_r8_typeless = 60,
    rf_r8_unorm = 61,
    rf_r8_uint = 62,
    rf_r8_snorm = 63,
    rf_r8_sint = 64,
    rf_a8_unorm = 65,
    rf_r1_unorm = 66,
    rf_r9g9b9e5_sharedexp = 67,
    rf_r8g8_b8g8_unorm = 68,
    rf_g8r8_g8b8_unorm = 69,
    rf_bc1_typeless = 70,
    rf_bc1_unorm = 71,
    rf_bc1_unorm_srgb = 72,
    rf_bc2_typeless = 73,
    rf_bc2_unorm = 74,
    rf_bc2_unorm_srgb = 75,
    rf_bc3_typeless = 76,
    rf_bc3_unorm = 77,
    rf_bc3_unorm_srgb = 78,
    rf_bc4_typeless = 79,
    rf_bc4_unorm = 80,
    rf_bc4_snorm = 81,
    rf_bc5_typeless = 82,
    rf_bc5_unorm = 83,
    rf_bc5_snorm = 84,
    rf_b5g6r5_unorm = 85,
    rf_b5g5r5a1_unorm = 86,
    rf_b8g8r8a8_unorm = 87,
    rf_b8g8r8x8_unorm = 88,
    rf_r10g10b10_xr_bias_a2_unorm = 89,
    rf_b8g8r8a8_typeless = 90,
    rf_b8g8r8a8_unorm_srgb = 91,
    rf_b8g8r8x8_typeless = 92,
    rf_b8g8r8x8_unorm_srgb = 93,
    rf_bc6h_typeless = 94,
    rf_bc6h_uf16 = 95,
    rf_bc6h_sf16 = 96,
    rf_bc7_typeless = 97,
    rf_bc7_unorm = 98,
    rf_bc7_unorm_srgb = 99,
    rf_ayuv = 100,
    rf_y410 = 101,
    rf_y416 = 102,
    rf_nv12 = 103,
    rf_p010 = 104,
    rf_p016 = 105,
    rf_420_opaque = 106,
    rf_yuy2 = 107,
    rf_y210 = 108,
    rf_y216 = 109,
    rf_nv11 = 110,
    rf_ai44 = 111,
    rf_ia44 = 112,
    rf_p8 = 113,
    rf_a8p8 = 114,
    rf_b4g4r4a4_unorm = 115,

    rf_p208 = 130,
    rf_v208 = 131,
    rf_v408 = 132,

    rf_sampler_feedback_min_mip_opaque = 189,
    rf_sampler_feedback_mip_region_used_opaque = 190,

    rf_force_uint = 0xffffffff
};

struct ClearColor {
    ResourceFormat format;
    bool isDepth{ false };
    float color[4];
    struct DSvalue {
        float depth;
        uint8_t stencil;
    } depth_tencil;
};

enum HeapType : uint32_t {
    ht_none                                 = 1 << 0,
    ht_default                              = 1 << 1,
    ht_upload                               = 1 << 2,
    ht_readback                             = 1 << 3,
    ht_custom                               = 1 << 4,
    ht_buff_transfer_src                    = 1 << 5,
    ht_buff_transfer_dst                    = 1 << 6,
    ht_buff_uniform_buffer                  = 1 << 7,
    ht_buff_index_buffer                    = 1 << 8,
    ht_buff_srv_buffer                      = 1 << 9,
    ht_image_sampled                        = 1 << 10,
    ht_image_depth_stencil_attachment       = 1 << 11,
    ht_image_storage                        = 1 << 12,
    ht_image_color_attach                   = 1 << 13,

    ht_aspect_color_bit                     = 1 << 24,
    ht_aspect_depth_bit                     = 1 << 25,
    ht_aspect_stencil_bit                   = 1 << 26
};

struct ResourceViewDesc {
    struct Texture2D {
        uint32_t most_detailed_mip;
        uint32_t mip_levels;
        float res_min_lod_clamp;
        uint32_t mip_slice;
    } texture2d;

    struct TextureCube {
        uint32_t most_detailed_mip;
        uint32_t mip_levels;
        float res_min_lod_clamp;
        uint32_t mip_slice;
    } texture_cube;

    struct Buffer {
        uint32_t first_element;
        uint32_t num_elements;
        uint32_t structure_byte_stride;
    } buffer;
};

struct SRVdesc : public ResourceViewDesc {
    ResourceFormat format;
    enum class SRVdimensionType {
        srv_dt_unknown = 0,
        srv_dt_buffer = 1,
        srv_dt_texture1d = 2,
        srv_dt_texture1darray = 3,
        srv_dt_texture2d = 4,
        srv_dt_texture2darray = 5,
        srv_dt_texture2dms = 6,
        srv_dt_texture2dmsarray = 7,
        srv_dt_texture3d = 8,
        srv_dt_texturecube = 9,
        srv_dt_texturecubearray = 10,
        srv_dt_raytracing_acceleration_structure = 11
    };
    SRVdimensionType dimension;
};

struct UAVdesc : public ResourceViewDesc {
    ResourceFormat format;
    enum class UAVdimensionType {
        uav_dt_unknown = 0,
        uav_dt_buffer = 1,
        uav_dt_texture1d = 2,
        uav_dt_texture1darray = 3,
        uav_dt_texture2d = 4,
        uav_dt_texture2darray = 5,
        uav_dt_texture2dms = 6,
        uav_dt_texture2dmsarray = 7,
        uav_dt_texture3d = 8
    };
    UAVdimensionType dimension;
};

struct CBVdesc {
    uint32_t size_in_bytes;
};

struct DSVdesc {
    ResourceFormat format;
    enum class DSVdimensionType {
        dsv_dt_unknown = 0,
        dsv_dt_texture1d = 1,
        dsv_dt_texture1darray = 2,
        dsv_dt_texture2d = 3,
        dsv_dt_texture2darray = 4,
        dsv_dt_texture2dms = 5,
        dsv_dt_texture2dmsarray = 6
    } dimension;
    enum class Flags {
        f_none = 0,
        f_read_only_depth = 0x1,
        f_read_only_stencil = 0x2
    };
    Flags flags{ Flags::f_none };
};

struct ResourceDesc {
    enum class ResourcesDimension {
        rd_unknown = 0,
        rd_buffer = 1,
        rd_texture1d = 2,
        rd_texture2d = 3,
        rd_texture3d = 4
    } resource_dimension;

    enum class TextureLayout {
        tl_unknown = 0,
        tl_row_major = 1,
        tl_64kb_undefined_swizzle = 2,
        tl_64kb_standard_swizzle = 3
    } texture_layout;

    enum ResourceFlags {
        rf_none = 0,
        rf_allow_render_target = 0x1,
        rf_allow_depth_stencil = 0x2,
        rf_allow_unordered_access = 0x4,
        rf_deny_shader_resource = 0x8,
        rf_allow_cross_adapter = 0x10,
        rf_allow_simultaneous_access = 0x20,
        rf_video_decode_reference_only = 0x40,
        rf_video_encode_reference_only = 0x80,
        rf_raytracing_acceleration_structure = 0x100
    } resource_flags;

    struct SampleDesc {
        uint32_t count;
        uint32_t quality;
    } sample_desc;

    struct ResourceAllocationInfo  {
        uint64_t size_in_bytes;
        uint64_t alignment;
    };

    uint64_t alignment;
    uint64_t width;
    uint32_t height;
    uint16_t depth_or_array_size;
    uint16_t mip_levels;
    ResourceFormat format;

    ResourceDesc() = default;

    ResourceDesc(
        ResourcesDimension dimension_,
        uint64_t alignment_,
        uint64_t width_,
        uint32_t height_,
        uint16_t depthOrArraySize,
        uint16_t mipLevels,
        ResourceFormat format_,
        uint32_t sampleCount,
        uint32_t sampleQuality,
        TextureLayout layout,
        ResourceFlags flags) noexcept
    {
        resource_dimension = dimension_;
        alignment = alignment_;
        width = width_;
        height = height_;
        depth_or_array_size = depthOrArraySize;
        mip_levels = mipLevels;
        format = format_;
        sample_desc.count = sampleCount;
        sample_desc.quality = sampleQuality;
        texture_layout = layout;
        resource_flags = flags;
    }

    static inline ResourceDesc tex_1d(
        ResourceFormat format,
        uint64_t width,
        uint16_t arraySize = 1,
        uint16_t mipLevels = 0,
        ResourceFlags flags = ResourceFlags::rf_none,
        TextureLayout layout = TextureLayout::tl_unknown,
        uint64_t alignment = 0) noexcept
    {
        return ResourceDesc(ResourcesDimension::rd_texture1d, alignment, width, 1, arraySize,
            mipLevels, format, 1, 0, layout, flags);
    }
    static inline ResourceDesc tex_2d(
        ResourceFormat format,
        uint64_t width,
        uint32_t height,
        uint16_t arraySize = 1,
        uint16_t mipLevels = 0,
        uint32_t sampleCount = 1,
        uint32_t sampleQuality = 0,
        ResourceFlags flags = ResourceFlags::rf_none,
        TextureLayout layout = TextureLayout::tl_unknown,
        uint64_t alignment = 0) noexcept
    {
        return ResourceDesc(ResourcesDimension::rd_texture2d, alignment, width, height, arraySize,
            mipLevels, format, sampleCount, sampleQuality, layout, flags);
    }
    static inline ResourceDesc tex_3d(
        ResourceFormat format,
        uint64_t width,
        uint32_t height,
        uint16_t depth,
        uint16_t mipLevels = 0,
        ResourceFlags flags = ResourceFlags::rf_none,
        TextureLayout layout = TextureLayout::tl_unknown,
        uint64_t alignment = 0) noexcept
    {
        return ResourceDesc(ResourcesDimension::rd_texture3d, alignment, width, height, depth,
            mipLevels, format, 1, 0, layout, flags);
    }
    static inline ResourceDesc Buffer(
        const ResourceAllocationInfo& resAllocInfo,
        ResourceFlags flags = ResourceFlags::rf_none) noexcept
    {
        return ResourceDesc(ResourcesDimension::rd_buffer, resAllocInfo.alignment, resAllocInfo.size_in_bytes,
            1, 1, 1, ResourceFormat::rf_unknown, 1, 0, TextureLayout::tl_row_major, flags);
    }
    static inline ResourceDesc Buffer(
        uint64_t width,
        ResourceFlags flags = ResourceFlags::rf_none,
        uint64_t alignment = 0) noexcept
    {
        return ResourceDesc(ResourcesDimension::rd_buffer, alignment, width, 1, 1, 1,
            ResourceFormat::rf_unknown, 1, 0, TextureLayout::tl_row_major, flags);
    }
};

struct SubresourceData {
    const void* data;
    uint64_t row_pitch;
    uint64_t slice_pitch;
};

// typedef SubresourceData SubresourceDataDx12;

struct SubresourceDataVk : public SubresourceData {
    uint32_t width;
    uint32_t height;
};

enum class CommandListType {
    clt_direct = 0,
    clt_bundle = 1,
    clt_compute = 2,
    clt_copy = 3,
    clt_video_decode = 4,
    clt_video_process = 5,
    clt_video_encode = 6,
    clt_none = -1
};

struct ViewPort {
    float topleft_x;
    float topleft_y;
    float width;
    float height;
    float min_depth;
    float max_depth;

    ViewPort(float topleft_x_, float topleft_y_, float width_, float height_, float min_depth_ = 0.f, float max_depth_ = 1.f) :
        topleft_x(topleft_x_),
        topleft_y(topleft_y_),
        width(width_),
        height(height_),
        min_depth(min_depth_),
        max_depth(max_depth_)
    {
    }

    ViewPort() = default;
};

struct RectScissors {
    uint32_t    left;
    uint32_t    top;
    uint32_t    right;
    uint32_t    bottom;

    RectScissors(uint32_t left_, uint32_t top_, uint32_t right_, uint32_t bottom_) :
        left(left_),
        top(top_),
        right(right_),
        bottom(bottom_)
    {

    }
    RectScissors() = default;
};

enum ClearFlagsDsv {
    cfdsv_depth = 0x1,
    cfdsv_stencil = 0x2
};
#define MAX_RTS_NUM 8

enum class PrimitiveTopology {
    pt_undefined = 0,
    pt_pointlist = 1,
    pt_linelist = 2,
    pt_linestrip = 3,
    pt_trianglelist = 4,
};

struct GPUdescriptor {
    uint64_t ptr;
    GPUdescriptor(uint64_t ptr_) :
        ptr(ptr_) {}
    GPUdescriptor() = default;
};

struct CPUdescriptor {
    uint64_t ptr;
    CPUdescriptor(uint64_t ptr_) :
        ptr(ptr_) {}
    CPUdescriptor() = default;
};

struct WindowHandler {
    uint64_t ptr;
    uint64_t instance;
    std::vector<const char*> extensions;
    void* callback;
};

struct ImguiWindowData {
    WindowHandler hWnd;
    uint32_t msg;
    uint64_t wParam;
    uint64_t lParam;
};

class IBackend;
IBackend* CreateBackendDx12();
void DestroyBackendDx12();
IBackend* CreateBackendVk();
void DestroyBackendVk();

enum class BackendType {
    bt_dx12,
    bt_vk
};

class IGpuResource;
extern IGpuResource* CreateGpuResourceDx();
extern IGpuResource* CreateGpuResourceVk();
inline IGpuResource* CreateGpuResource(BackendType bk_type) {
    if (bk_type == BackendType::bt_dx12) {
        return CreateGpuResourceDx();
    }
    else {
        return CreateGpuResourceVk();
    }
}

enum BindingId {
    bi_model_cb = 0,
    bi_g_buffer_tex_table = 1,
    bi_scene_cb = 2,
    bi_vertex_buffer = 4,
    bi_materials_cb = 3,
    bi_lights_cb = 3,
    bi_deferred_shading_tex_table = 1,
    bi_post_proc_input_tex_table = 0,
    bi_ssao_cb = 0,
    bi_ssao_input_tex = 1,
    bi_ssao_uav_tex = 3,
    bi_terrain_hm = 1,
    bi_fwd_tex = 1,
    bi_refl_srv = 1,
    bi_refl_uav = 3,
};

enum ConstantBuffers {
    cb_model = 0,
    cb_scene = 1,
    cb_lights = 2,
    cb_materials = 3,
    cb_ssao = 4,
};

enum TextureTableOffset {
    tto_albedo = 0,
    tto_normals = 1,
    tto_metallic = 2,
    tto_roughness = 3,
    tto_gbuff_albedo = 0,
    tto_gbuff_normals = 1,
    tto_gbuff_positions = 2,
    tto_gbuff_materials = 3,
    tto_gbuff_ssao = 4,
    tto_gbuff_sun_sm = 5,
    tto_postp_input = 0,
    tto_postp_gui = 1,
    tto_postp_fwd = 2,
    tto_postp_ssao = 3,
    tto_postp_sun_sm = 4,
    tto_postp_ssr = 5,
    tto_ssao_depth = 0,
    tto_ssao_normals = 1,
    tto_ssao_random_vals = 2,
    tto_ssao_positions = 3,
    tto_ssao_blur_srv = 0,
    tto_ssao_blur_uav = 0,
    tto_fwd_skybox = 0,
    tto_vertex_buffer = 5,
    tto_refl_normals = 0,
    tto_refl_colors = 1,
    tto_refl_materials = 2,
    tto_refl_world_poses = 3,
    tto_refl_uav = 0,
};
namespace vk_hlsl_bindings {
    const uint32_t cbv_offset = 10;
    const uint32_t srv_offset = 20;
    const uint32_t uav_offset = 30;
}
