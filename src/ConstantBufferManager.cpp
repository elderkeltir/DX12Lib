#include "ConstantBufferManager.h"
#include "IGpuResource.h"
#include "RenderHelper.h"
#include "ICommandQueue.h"
#include "ICommandList.h"
#include "IHeapBuffer.h"
#include "Frontend.h"
#include "defines.h"

extern Frontend* gFrontend;


void ConstantBufferManager::OnInit() {
	for (uint32_t i = 0; i < 2; i++) {
        m_scene_cbs[i].reset(CreateGpuResource());
        IGpuResource& m_scene_cb = *m_scene_cbs[i];
        HeapType h_type = HeapType(HeapType::ht_upload | HeapType::ht_buff_uniform_buffer);
		m_scene_cb.CreateBuffer(h_type, calc_cb_size(sizeof(SceneCB)), ResourceState::rs_resource_state_generic_read, L"Scene_cb");
		CBVdesc desc;
		desc.size_in_bytes = calc_cb_size(sizeof(SceneCB));
		m_scene_cb.Create_CBV(desc);
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cb.GetBuffer().lock()) {
			buff->Map();
		}
	}
}

void ConstantBufferManager::Destroy() {
    const uint32_t id = gFrontend->FrameId();
    if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[id]->GetBuffer().lock()) {
        buff->Unmap();
    }
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix){
    const uint32_t frame_id = gFrontend->FrameId();
    if (id == Constants::cM){
        if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&model_cb->M, matrix);
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->V, matrix);
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->P, matrix);
        }
    }
	else if (id == Constants::cPinv) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4x4(&scene_cb->VPinv, matrix);
		}
	}
    else if (id == Constants::cSunV) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->SunV, matrix);
        }
    }
    else if (id == Constants::cSunP) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->SunP, matrix);
        }
    }
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix){
    const uint32_t frame_id = gFrontend->FrameId();
    if (id == Constants::cM){
        if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            model_cb->M = matrix;
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->V = matrix;
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->P = matrix;
        }
    }
    else if (id == Constants::cPinv) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->VPinv = matrix;
		}
    }
    else if (id == Constants::cSunV) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            scene_cb->SunV = matrix;
        }
    }
    else if (id == Constants::cSunP) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            scene_cb->SunP = matrix;
        }
    }
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec){
    const uint32_t frame_id = gFrontend->FrameId();
    if (id == Constants::cCP){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4(&scene_cb->CamPos, vec);
        }
    }
    else if (id == Constants::cCD) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            DirectX::XMStoreFloat4(&scene_cb->CamDir, vec);
        }
    }
	else if (id == Constants::cRTdim) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->RTdim, vec);
		}
	}
	else if (id == Constants::cNearFar) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->NearFarZ, vec);
		}
	}
	else if (id == Constants::cTime) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->Time, vec);
		}
	}
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec){
    const uint32_t frame_id = gFrontend->FrameId();
    if (id == Constants::cCP){
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->CamPos = vec;
        }
    }
    else if (id == Constants::cCD) {
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            scene_cb->CamDir = vec;
        }
    }
	else if (id == Constants::cRTdim) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            scene_cb->RTdim = vec;
		}
	}
	else if (id == Constants::cNearFar) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->NearFarZ = vec;
		}
	}
	else if (id == Constants::cTime) {
		if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->Time = vec;
		}
	}
}

void ConstantBufferManager::SetUint32(Constants id, uint32_t val)
{
    if (id == Constants::cMat) {
		if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
			ModelCB* model_cb = (ModelCB*)buff->GetCpuData();
			model_cb->material_id = val;
		}
    }
    else if (id == Constants::cVertexBufferOffset) {
        if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
            ModelCB* model_cb = (ModelCB*)buff->GetCpuData();
            model_cb->vertex_buffer_offset = val;
        }
    }
    else if (id == Constants::cVertexType) {
        if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
            ModelCB* model_cb = (ModelCB*)buff->GetCpuData();
            model_cb->vertex_type = val;
        }
    }
}

void ConstantBufferManager::CommitCB(ICommandList* command_list, ConstantBuffers id, bool gfx)
{
    if (id == cb_model) {
        if (std::shared_ptr<IHeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
            if (gfx) {
                command_list->SetGraphicsRootConstantBufferView(bi_model_cb, buff);
            }
            else {
                command_list->SetComputeRootConstantBufferView(bi_model_cb, buff);
            }
        }
    }
    else if (id == cb_scene) {
        const uint32_t frame_id = gFrontend->FrameId();
        if (std::shared_ptr<IHeapBuffer> buff = m_scene_cbs[frame_id]->GetBuffer().lock()) {
            if (gfx) {
                command_list->SetGraphicsRootConstantBufferView(bi_scene_cb, buff);
            }
            else {
                command_list->SetComputeRootConstantBufferView(bi_scene_cb, buff);
            }
        }
    }
}

void ConstantBufferManager::SyncCpuDataToCB(ICommandList* command_list, IGpuResource* res, void* cpu_data, uint32_t size, BindingId bind_point, bool gfx) {
    if (gfx) {
        command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_copy_dest);
    }
    else {
	    command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_copy_dest);
    }

	if (std::shared_ptr<IHeapBuffer> buff = res->GetBuffer().lock()) {
		uint32_t cb_size = calc_cb_size(size);
		buff->Load(command_list, 1, cb_size, cpu_data);
	}

    if (gfx) {
        command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_vertex_and_constant_buffer);
    }
    else {
	    command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_vertex_and_constant_buffer);
    }

	if (std::shared_ptr<IHeapBuffer> buff = res->GetBuffer().lock()) {
        if (gfx) {
            command_list->SetGraphicsRootConstantBufferView(bind_point, buff);
        }
        else {
            command_list->SetComputeRootConstantBufferView(bind_point, buff);
        }
	}
}
