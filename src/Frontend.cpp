#include "Frontend.h"
#include <cassert>
#include "defines.h"
#include "IBackend.h"
#include "FreeCamera.h"
#include "Level.h"
#include "ICommandList.h"
#include "SSAO.h"
#include "RenderQuad.h"
#include "IDynamicGpuHeap.h"
#include "IResourceDescriptor.h"
#include "Reflections.h"
#include "Sun.h"
#include "ICommandQueue.h"
#include "IImguiHelper.h"
#include "MaterialManager.h"
#include "GpuDataManager.h"

Frontend* gFrontend = nullptr;

#define BEGIN_EVENT(cmd_list, name) m_backend->DebugSectionBegin(cmd_list, name)
#define END_EVENT(cmd_list) m_backend->DebugSectionEnd(cmd_list)

Frontend::Frontend(uint32_t width, uint32_t height, std::wstring name) : 
	m_width(width),
	m_height(height),                                                   
	m_title(name),
	m_post_process_quad(std::make_unique<RenderQuad>()),
	m_forward_quad(std::make_unique<RenderQuad>()),
	m_deferred_shading_quad(std::make_unique<RenderQuad>()),
	m_ssao(std::make_unique<SSAO>()),
	m_reflections(std::make_unique<Reflections>())
{
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

Frontend::~Frontend()
{
}

void Frontend::OnInit(const WindowHandler& hwnd, const std::filesystem::path &root_dir)
{
	m_start_time = std::chrono::system_clock::now();
	gFrontend = this;

	// create backend
	ResourceManager::OnInit(root_dir);
	m_backend.reset(CreateBackend());
	m_backend->OnInit(hwnd, m_width, m_height, root_dir);

	ConstantBufferManager::OnInit();

	m_level = std::make_shared<Level>();
	m_level->Load(L"test_level.json");
	m_material_mgr->LoadMaterials();

	m_post_process_quad->Initialize();
	m_deferred_shading_quad->Initialize();
	m_forward_quad->Initialize();
	m_ssao->Initialize(m_width, m_height, L"SSAO_");
	m_reflections->Initialize();

	m_gpu_data_mgr->Initialize();
}

void Frontend::OnUpdate()
{
	// Time logic
	m_frame_id++;
	std::chrono::time_point t = std::chrono::system_clock::now();
	m_dt = t - m_time;
	m_time = t;
	m_total_time = t - m_start_time;

	if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()) {
		UpdateCamera(camera, m_dt.count());
	}

	m_level->Update(m_dt.count());
}


void Frontend::OnRender()
{
	// Wait for signal from new frame
	m_backend->SyncWithCPU();

	m_backend->ChechUpdatedShader();

	// Gui
	m_backend->RenderUI();
	
	// Record all the commands we need to render the scene into the command list.
	ICommandList* command_list_gfx = m_backend->InitCmdList();

	// Render scene
	BEGIN_EVENT(command_list_gfx, "G-Buffer");
	RenderLevel(command_list_gfx);
	command_list_gfx->ResourceBarrier(*m_ssao->GetSSAOres(2), ResourceState::rs_resource_state_unordered_access);
	END_EVENT(command_list_gfx);

	// send G-buffer to execute
	GetGfxQueue()->ExecuteActiveCL();
	
	m_backend->SyncWithGpu(ICommandQueue::QueueType::qt_gfx, ICommandQueue::QueueType::qt_compute);

	// shadow map
	command_list_gfx = m_backend->InitCmdList();
	BEGIN_EVENT(command_list_gfx, "ShadowMap");
	m_level->RenderShadowMap(command_list_gfx);
	END_EVENT(command_list_gfx);

	GetGfxQueue()->ExecuteActiveCL();

	ICommandList* command_list_compute = GetComputeQueue()->ResetActiveCL();

	BEGIN_EVENT(command_list_compute, "SSAO");
	RenderSSAOquad(command_list_compute);
	BlurSSAO(command_list_compute);
	END_EVENT(command_list_compute);

	// send SSAO to execute
	GetComputeQueue()->ExecuteActiveCL();
	m_backend->SyncWithGpu(ICommandQueue::QueueType::qt_compute, ICommandQueue::QueueType::qt_gfx);

	command_list_gfx = m_backend->InitCmdList();

	// deferred shading
	BEGIN_EVENT(command_list_gfx, "Deferred Shading");
	RenderDeferredShadingQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// ssr
	BEGIN_EVENT(command_list_gfx, "SSR");
	GenerateReflections(command_list_gfx);
	END_EVENT(command_list_gfx);

	// forward pass
	BEGIN_EVENT(command_list_gfx, "Forward Pass");
	RenderForwardQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// post process
	BEGIN_EVENT(command_list_gfx, "Post Processing");
	RenderPostProcessQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// Indicate that the back buffer will now be used to present.
	command_list_gfx->ResourceBarrier(GetCurrentRT(), ResourceState::rs_resource_state_present);
	GetGfxQueue()->ExecuteActiveCL();

	// Present the frame.
	m_backend->Present();
}

void Frontend::OnDestroy()
{
	gFrontend = nullptr;
}

void Frontend::OnKeyDown(KeyboardButton key) {
	if (key == KeyboardButton::kb_none)
		return;

	if (key != KeyboardButton::kb_tilda && m_backend->GetUI()->WantCapture(IImguiHelper::CaptureInput_type::cit_keyboard)) {
		return;
	}
	switch (key) {
	case KeyboardButton::kb_w:
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_fwd;
		break;
	case KeyboardButton::kb_a:
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_left;
		break;
	case KeyboardButton::kb_s:
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_bcwd;
		break;
	case KeyboardButton::kb_d:
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_right;
		break;
	case KeyboardButton::kb_tilda:
		m_backend->GetUI()->ShowConsole();
		break;
	}
}

void Frontend::OnKeyUp(KeyboardButton key) {
	if (key == KeyboardButton::kb_none)
		return;

	if (m_backend->GetUI()->WantCapture(IImguiHelper::CaptureInput_type::cit_keyboard)) {
		return;
	}
	switch (key) {
	case KeyboardButton::kb_w:
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_fwd);
		break;
	case KeyboardButton::kb_a:
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_left);
		break;
	case KeyboardButton::kb_s:
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_bcwd);
		break;
	case KeyboardButton::kb_d:
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_right);
		break;
	}
}

void Frontend::OnMouseMoved(MouseButton bnt, int x, int y) {
	if (bnt == MouseButton::mb_right) {
		m_camera_movement.camera_x_delta = x - m_camera_movement.camera_x;
		m_camera_movement.camera_y_delta = y - m_camera_movement.camera_y;
	}

	m_camera_movement.camera_x = x;
	m_camera_movement.camera_y = y;
}

uint32_t Frontend::FrameId() const
{
	return m_backend->GetCurrentBackBufferIndex();
}

uint32_t Frontend::GetRenderMode() const
{
	return m_backend->GetRenderMode();
}

void Frontend::UpdateCamera(std::shared_ptr<FreeCamera>& camera, float dt) {
	auto move_func = [&camera, dt](const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, bool neg) {
		const float camera_speed = 30;
		const DirectX::XMVECTOR dir_vec = neg ? DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&dir)) : DirectX::XMLoadFloat3(&dir);
		const DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&pos);
		auto mx = DirectX::XMFLOAT3(camera_speed * dt, camera_speed * dt, camera_speed * dt);
		DirectX::XMVECTOR new_pos_vec = DirectX::XMVectorMultiplyAdd(dir_vec, DirectX::XMLoadFloat3(&mx), pos_vec);
		camera->Move(new_pos_vec);
	};

	if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd)) {
		move_func(camera->GetPosition(), camera->GetDirection(), false);
	}
	else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd)) {
		move_func(camera->GetPosition(), camera->GetDirection(), true);
	}
	if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_right) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_left)) {
		move_func(camera->GetPosition(), camera->GetRightDirection(), false);
	}
	else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_left) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_right)) {
		move_func(camera->GetPosition(), camera->GetRightDirection(), true);
	}

	auto rot_func = [&camera, dt](const DirectX::XMFLOAT3& dir, const DirectX::XMFLOAT3& rot_axis, bool x, uint32_t w, uint32_t h, int32_t dx, int32_t dy) {

		const float ang = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x ? dx : dy));

		const DirectX::XMVECTOR dir_vec = DirectX::XMLoadFloat3(&dir);
		const DirectX::XMVECTOR axis_vec = DirectX::XMLoadFloat3(&rot_axis);

		DirectX::XMMATRIX rot_mx = DirectX::XMMatrixRotationAxis(axis_vec, ang);
		const DirectX::XMVECTOR new_dir = DirectX::XMVector3TransformNormal(dir_vec, rot_mx);

		camera->Rotate(new_dir);
	};

	if (m_camera_movement.camera_x_delta != 0) {
		rot_func(camera->GetDirection(), camera->GetUpDirection(), true, m_width, m_height, m_camera_movement.camera_x_delta, 0);
		m_camera_movement.camera_x_delta = 0;
	}

	if (m_camera_movement.camera_y_delta != 0) {
		rot_func(camera->GetDirection(), camera->GetRightDirection(), false, m_width, m_height, 0, m_camera_movement.camera_y_delta);
		m_camera_movement.camera_y_delta = 0;
	}
}

void Frontend::PrepareRenderTarget(ICommandList* command_list, const std::vector<std::shared_ptr<IGpuResource>>& rts, bool set_dsv, bool clear_dsv) {
	std::vector<IGpuResource*> render_targets(rts.size());
	for (uint32_t i = 0; i < rts.size(); i++) {
		render_targets[i] = rts[i].get();
	}

	if (set_dsv) {
		if (clear_dsv) {
			command_list->ClearDepthStencilView(GetDepthBuffer(), ClearFlagsDsv::cfdsv_depth, 1.0f, 0, 0, nullptr);
		}

		command_list->SetRenderTargets(render_targets, GetDepthBuffer());
	}
	else {
		command_list->SetRenderTargets(render_targets, nullptr);
	}

	// Record commands.
	const float clearColor[] = { 0.f, 0.f, 0.f, 0.0f };
	for (auto& rt : rts) {
		command_list->ClearRenderTargetView(rt.get(), clearColor, 0, nullptr);
	}
}

void Frontend::PrepareRenderTarget(ICommandList* command_list, IGpuResource& rt, bool set_dsv, bool clear_dsv) {
	std::vector<IGpuResource*> rts{ &rt };

	if (set_dsv) {
		command_list->SetRenderTargets(rts, GetDepthBuffer());
		if (clear_dsv) {
			command_list->ClearDepthStencilView(rt, ClearFlagsDsv::cfdsv_depth, 1.0f, 0, 0, nullptr);
		}
	}
	else {
		command_list->SetRenderTargets(rts, nullptr);
	}

	// Record commands.
	const float clearColor[] = { 0.f, 0.f, 0.f, 0.0f };
	command_list->ClearRenderTargetView(rt, clearColor, 0, nullptr);
}

void Frontend::RenderLevel(ICommandList* command_list) {
	std::vector<ResourceFormat> formats = { ResourceFormat::rf_r16g16b16a16_float, ResourceFormat::rf_r16g16b16a16_float, ResourceFormat::rf_r16g16b16a16_float, ResourceFormat::rf_r16g16b16a16_float };
	m_deferred_shading_quad->CreateQuadTexture(m_width, m_height, formats, m_backend->GetFrameCount(), 0, L"m_deferred_shading_quad_");
	{
		std::vector<std::shared_ptr<IGpuResource>>& rts = m_deferred_shading_quad->GetRts(FrameId());
		command_list->ResourceBarrier(rts, ResourceState::rs_resource_state_render_target);
		PrepareRenderTarget(command_list, rts);
	}

	m_level->Render(command_list);

	command_list->ResourceBarrier(*GetDepthBuffer(), ResourceState::rs_resource_state_depth_read | ResourceState::rs_resource_state_non_pixel_shader_resource);
	{
		std::vector< std::shared_ptr<IGpuResource>>& rts = m_deferred_shading_quad->GetRts(FrameId());
		for (uint32_t i = 0; i < rts.size(); i++) {
			if (i == 1 || i == 2) {
				command_list->ResourceBarrier(rts[i], ResourceState::rs_resource_state_non_pixel_shader_resource);
			}
			else {
				command_list->ResourceBarrier(rts[i], ResourceState::rs_resource_state_pixel_shader_resource);
			}
		}
	}
}

void Frontend::RenderPostProcessQuad(ICommandList* command_list) {
	if (std::shared_ptr<IGpuResource> rt = m_post_process_quad->GetRt(FrameId()).lock()) {
		command_list->ResourceBarrier(rt, ResourceState::rs_resource_state_pixel_shader_resource);
	}
	else {
		assert(false);
	}

	command_list->ResourceBarrier(GetCurrentRT(), ResourceState::rs_resource_state_render_target);
	PrepareRenderTarget(command_list, GetCurrentRT(), false);

	const ITechniques::Technique* tech = GetTechniqueById(ITechniques::TecnhinueType::tt_post_processing);
	if (command_list->GetPSO() != ITechniques::TecnhinueType::tt_post_processing) {
		command_list->SetPSO(ITechniques::TecnhinueType::tt_post_processing);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature);
		command_list->GetQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));
	}

	if (std::shared_ptr<IGpuResource> rt = m_post_process_quad->GetRt(FrameId()).lock()) {
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_input, srv);
		}
	}

	if (IGpuResource* rt = m_backend->GetUI()->GetGuiQuad(FrameId())) {
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_gui, srv);
		}
	}

	if (std::shared_ptr<IGpuResource> rt = m_forward_quad->GetRt(FrameId()).lock()) {
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_fwd, srv);
		}
	}

	if (std::shared_ptr<IResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_ssao, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> srv = m_level->GetSunShadowMap().GetSRV().lock()) {
		command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_sun_sm, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> srv = m_reflections->GetReflectionMap().GetSRV().lock()) {
		command_list->ResourceBarrier(m_reflections->GetReflectionMap(), ResourceState::rs_resource_state_pixel_shader_resource);
		command_list->GetQueue()->GetGpuHeap().StageDesctriptorInTable(bi_post_proc_input_tex_table, tto_postp_ssr, srv);
	}

	CommitCB(command_list, cb_scene);

	command_list->GetQueue()->GetGpuHeap().CommitRootSignature(command_list);

	m_post_process_quad->Render(command_list);
}

void Frontend::RenderForwardQuad(ICommandList* command_list) {
	{
		std::vector<ResourceFormat> formats = { ResourceFormat::rf_r16g16b16a16_float };
		m_forward_quad->CreateQuadTexture(m_width, m_height, formats, m_backend->GetFrameCount(), 0, L"m_forward_quad_");
	}

	if (std::shared_ptr<IGpuResource> rt = m_forward_quad->GetRt(FrameId()).lock()) {
		command_list->ResourceBarrier(rt, ResourceState::rs_resource_state_render_target);
		PrepareRenderTarget(command_list, *rt.get(), true, false);
	}
	else {
		assert(false);
	}

	// water
	m_level->RenderWater(command_list);

	if (std::shared_ptr<IGpuResource> rt = m_forward_quad->GetRt(FrameId()).lock()) {
		command_list->ResourceBarrier(rt, ResourceState::rs_resource_state_pixel_shader_resource);
	}

	//m_post_process_quad->Render(command_list);
}

void Frontend::RenderDeferredShadingQuad(ICommandList* command_list) {
	command_list->ResourceBarrier(*m_ssao->GetSSAOres(2), ResourceState::rs_resource_state_pixel_shader_resource);
	command_list->ResourceBarrier(*GetDepthBuffer(), ResourceState::rs_resource_state_depth_write);

	{
		std::vector< std::shared_ptr<IGpuResource>>& rts = m_deferred_shading_quad->GetRts(FrameId());
		for (uint32_t i = 0; i < rts.size(); i++) {
			if (i == 1 || i == 2) {
				command_list->ResourceBarrier(rts[i], ResourceState::rs_resource_state_pixel_shader_resource);
			}
		}
	}

	{
		std::vector<ResourceFormat> formats = { ResourceFormat::rf_r16g16b16a16_float };
		m_post_process_quad->CreateQuadTexture(m_width, m_height, formats, m_backend->GetFrameCount(), 0, L"m_post_process_quad_");
		if (std::shared_ptr<IGpuResource> rt = m_post_process_quad->GetRt(FrameId()).lock()) {
			command_list->ResourceBarrier(rt, ResourceState::rs_resource_state_render_target);
			PrepareRenderTarget(command_list, m_post_process_quad->GetRts(FrameId()), false);
		}
		else {
			assert(false);
		}
	}

	// set technique
	const ITechniques::Technique* tech = GetTechniqueById(ITechniques::TecnhinueType::tt_deferred_shading);
	if (command_list->GetPSO() != ITechniques::TecnhinueType::tt_deferred_shading) {
		command_list->SetPSO(ITechniques::TecnhinueType::tt_deferred_shading);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature);
		GetGfxQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));
	}

	CommitCB(command_list, cb_scene);
	m_level->BindLights(command_list);

	if (std::shared_ptr<IResourceDescriptor> srv = m_level->GetSunShadowMap().GetSRV().lock()) {
		command_list->ResourceBarrier(m_level->GetSunShadowMap(), ResourceState::rs_resource_state_pixel_shader_resource | ResourceState::rs_resource_state_depth_read);
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_deferred_shading_tex_table, tto_gbuff_sun_sm, srv);
	}

	auto& rts_vec = m_deferred_shading_quad->GetRts(FrameId());
	for (uint32_t i = 0; i < rts_vec.size(); i++) {
		std::shared_ptr<IGpuResource>& rt = rts_vec[i];
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_deferred_shading_tex_table, TextureTableOffset(i), srv);
		}
	}

	if (std::shared_ptr<IResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_deferred_shading_tex_table, tto_gbuff_ssao, srv);
	}

	GetGfxQueue()->GetGpuHeap().CommitRootSignature(command_list);

	m_deferred_shading_quad->Render(command_list);
}

void Frontend::RenderSSAOquad(ICommandList* command_list) {
	command_list->ResourceBarrier(*m_ssao->GetSSAOres(0), ResourceState::rs_resource_state_unordered_access);
	const ITechniques::Technique* tech = GetTechniqueById(ITechniques::TecnhinueType::tt_ssao);
	if (command_list->GetPSO() != ITechniques::TecnhinueType::tt_ssao) {
		command_list->SetPSO(ITechniques::TecnhinueType::tt_ssao);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature);
		GetComputeQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));
	}

	if (std::shared_ptr<IGpuResource> rt = m_deferred_shading_quad->GetRt(FrameId(), 1).lock()) {
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_normals, srv);
		}
	}
	if (std::shared_ptr<IGpuResource> rt = m_deferred_shading_quad->GetRt(FrameId(), 2).lock()) {
		if (std::shared_ptr<IResourceDescriptor> srv = rt->GetSRV().lock()) {
			GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_positions, srv);
		}
	}
	if (std::shared_ptr<IResourceDescriptor> srv = m_ssao->GetRandomVals()->GetSRV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_random_vals, srv);
	}
	if (std::shared_ptr<IResourceDescriptor> depth_view = GetDepthBuffer()->GetSRV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_depth, depth_view);
	}
	if (std::shared_ptr<IResourceDescriptor> uav = m_ssao->GetSSAOres(0)->GetUAV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_uav_tex, tto_ssao_blur_uav, uav);
	}

	CommitCB(command_list, cb_scene, false);
	// bind ssao cb
	m_ssao->GenerateSSAO(command_list, false);

	GetComputeQueue()->GetGpuHeap().CommitRootSignature(command_list, false);

	const float threads_num = 32.f;
	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);

	command_list->ResourceBarrier(*m_ssao->GetSSAOres(0), ResourceState::rs_resource_state_non_pixel_shader_resource);
	command_list->ResourceBarrier(*m_ssao->GetSSAOres(1), ResourceState::rs_resource_state_unordered_access);
}

void Frontend::BlurSSAO(ICommandList* command_list)
{
	const ITechniques::Technique* tech = GetTechniqueById(ITechniques::TecnhinueType::tt_blur);
	if (command_list->GetPSO() != ITechniques::TecnhinueType::tt_blur) {
		command_list->SetPSO(ITechniques::TecnhinueType::tt_blur);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature);
	}
	GetComputeQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));

	if (std::shared_ptr<IResourceDescriptor> srv = m_ssao->GetSSAOres(0)->GetSRV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_blur_srv, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> uav = m_ssao->GetSSAOres(1)->GetUAV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_uav_tex, tto_ssao_blur_uav, uav);
	}
	// bind ssao cb
	//m_ssao->GenerateSSAO(command_list, false);

	GetComputeQueue()->GetGpuHeap().CommitRootSignature(command_list, false);
	const float threads_num = 32.f;
	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);

	// vertical pass
	GetComputeQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));

	command_list->ResourceBarrier(*m_ssao->GetSSAOres(1), ResourceState::rs_resource_state_non_pixel_shader_resource);


	if (std::shared_ptr<IResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_input_tex, tto_ssao_blur_srv, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> uav = m_ssao->GetSSAOres(2)->GetUAV().lock()) {
		GetComputeQueue()->GetGpuHeap().StageDesctriptorInTable(bi_ssao_uav_tex, tto_ssao_blur_uav, uav);
	}

	GetComputeQueue()->GetGpuHeap().CommitRootSignature(command_list, false);

	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);
}

void Frontend::GenerateReflections(ICommandList* command_list)
{
	const ITechniques::Technique* tech = GetTechniqueById(ITechniques::TecnhinueType::tt_reflection_map);
	if (command_list->GetPSO() != ITechniques::TecnhinueType::tt_reflection_map) {
		command_list->SetPSO(ITechniques::TecnhinueType::tt_reflection_map);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature, false);
	}
	GetGfxQueue()->GetGpuHeap().CacheRootSignature(GetRootSignById(tech->root_signature));

	// resources
	std::vector< std::shared_ptr<IGpuResource>>& rts = m_deferred_shading_quad->GetRts(FrameId());
	for (uint32_t i = 1; i < 4; i++) {
		command_list->ResourceBarrier(rts[i], ResourceState::rs_resource_state_non_pixel_shader_resource);
	}
	command_list->ResourceBarrier(m_post_process_quad->GetRts(FrameId()).at(0), ResourceState::rs_resource_state_non_pixel_shader_resource);

	if (std::shared_ptr<IResourceDescriptor> srv = m_post_process_quad->GetRts(FrameId()).at(0)->GetSRV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_refl_srv, tto_refl_colors, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> srv = rts[1]->GetSRV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_refl_srv, tto_refl_normals, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> srv = rts[2]->GetSRV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_refl_srv, tto_refl_world_poses, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> srv = rts[3]->GetSRV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_refl_srv, tto_refl_materials, srv);
	}

	if (std::shared_ptr<IResourceDescriptor> uav = m_reflections->GetReflectionMap().GetUAV().lock()) {
		GetGfxQueue()->GetGpuHeap().StageDesctriptorInTable(bi_refl_uav, tto_refl_uav, uav);
	}

	CommitCB(command_list, cb_scene, false);
	GetGfxQueue()->GetGpuHeap().CommitRootSignature(command_list, false);

	// dispatch
	const float threads_num = 32.f;
	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);
}

IGpuResource& Frontend::GetCurrentRT()
{
	return m_backend->GetCurrentBackBuffer();
}

IGpuResource* Frontend::GetDepthBuffer()
{
	return m_backend->GetDepthBuffer();
}

ICommandQueue* Frontend::GetGfxQueue()
{
	return m_backend->GetQueue(ICommandQueue::QueueType::qt_gfx).get();
}

ICommandQueue* Frontend::GetComputeQueue()
{
	return m_backend->GetQueue(ICommandQueue::QueueType::qt_compute).get();
}

bool Frontend::PassImguiWndProc(const ImguiWindowData& data)
{
	if (m_backend)
		return m_backend->PassImguiWndProc(data);

	return false;
}

bool Frontend::ShouldClose() const
{
	return m_backend->ShouldClose();
}

const ITechniques::Technique* Frontend::GetTechniqueById(uint32_t id) const
{
	return m_backend->GetTechniqueById(id);
}

const IRootSignature* Frontend::GetRootSignById(uint32_t id)
{
	return m_backend->GetRootSignById(id);
}
