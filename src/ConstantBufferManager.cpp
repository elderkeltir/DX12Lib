#include "ConstantBufferManager.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "GfxCommandQueue.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;


void ConstantBufferManager::OnInit() {
	for (uint32_t i = 0; i < 2; i++) {
        GpuResource& m_scene_cb = m_scene_cbs[i];
		m_scene_cb.CreateBuffer(HeapBuffer::BufferType::bt_upload, calc_cb_size(sizeof(SceneCB)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_GENERIC_READ, L"Scene_cb");
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.SizeInBytes = calc_cb_size(sizeof(SceneCB));
		m_scene_cb.Create_CBV(desc);
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cb.GetBuffer().lock()) {
			buff->Map();
		}
	}
}

void ConstantBufferManager::Destroy() {
    const uint32_t id = gD3DApp->FrameId();
    if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[id].GetBuffer().lock()) {
        buff->Unmap();
    }
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix){
    const uint32_t frame_id = gD3DApp->FrameId();
    if (id == Constants::cM){
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&model_cb->M, matrix);
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->V, matrix);
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->P, matrix);
        }
    }
	else if (id == Constants::cPinv) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4x4(&scene_cb->VPinv, matrix);
		}
	}
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix){
    const uint32_t frame_id = gD3DApp->FrameId();
    if (id == Constants::cM){
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            model_cb->M = matrix;
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->V = matrix;
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->P = matrix;
        }
    }
    else if (id == Constants::cPinv) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->VPinv = matrix;
		}
    }
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec){
    const uint32_t frame_id = gD3DApp->FrameId();
    if (id == Constants::cCP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4(&scene_cb->CamPos, vec);
        }
    }
	else if (id == Constants::cRTdim) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->RTdim, vec);
		}
	}
	else if (id == Constants::cNearFar) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->NearFarZ, vec);
		}
	}
	else if (id == Constants::cTime) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			DirectX::XMStoreFloat4(&scene_cb->Time, vec);
		}
	}
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec){
    const uint32_t frame_id = gD3DApp->FrameId();
    if (id == Constants::cCP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->CamPos = vec;
        }
    }
	else if (id == Constants::cRTdim) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
            scene_cb->RTdim = vec;
		}
	}
	else if (id == Constants::cNearFar) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->NearFarZ = vec;
		}
	}
	else if (id == Constants::cTime) {
		if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
			SceneCB* scene_cb = (SceneCB*)buff->GetCpuData();
			scene_cb->Time = vec;
		}
	}
}

void ConstantBufferManager::SetUint32(Constants id, uint32_t val)
{
    if (id == Constants::cMat) {
		if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
			ModelCB* model_cb = (ModelCB*)buff->GetCpuData();
			model_cb->material_id = val;
		}
    }
}

void ConstantBufferManager::CommitCB(CommandList& command_list, ConstantBuffers id, bool gfx)
{
    if (id == cb_model) {
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
            if (gfx) {
                command_list.SetGraphicsRootConstantBufferView(bi_model_cb, buff->GetResource()->GetGPUVirtualAddress());
            }
            else {
                command_list.SetComputeRootConstantBufferView(bi_model_cb, buff->GetResource()->GetGPUVirtualAddress());
            }
        }
    }
    else if (id == cb_scene) {
        const uint32_t frame_id = gD3DApp->FrameId();
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cbs[frame_id].GetBuffer().lock()) {
            if (gfx) {
                command_list.SetGraphicsRootConstantBufferView(bi_scene_cb, buff->GetResource()->GetGPUVirtualAddress());
            }
            else {
                command_list.SetComputeRootConstantBufferView(bi_scene_cb, buff->GetResource()->GetGPUVirtualAddress());
            }
        }
    }
}

void ConstantBufferManager::SyncCpuDataToCB(CommandList& command_list, GpuResource* res, void* cpu_data, uint32_t size, BindingId bind_point, bool gfx) {
    if (gfx) {
        if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()) {
            queue->ResourceBarrier(*res, D3D12_RESOURCE_STATE_COPY_DEST);
        }
    }
    else {
		if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetComputeQueue().lock()) {
			queue->ResourceBarrier(*res, D3D12_RESOURCE_STATE_COPY_DEST);
		}
    }

	if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()) {
		uint32_t cb_size = calc_cb_size(size);
		buff->Load(command_list, 1, cb_size, cpu_data);
	}

    if (gfx) {
        if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()) {
            queue->ResourceBarrier(*res, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        }
    }
    else {
		if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetComputeQueue().lock()) {
			queue->ResourceBarrier(*res, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
    }

	if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()) {
        if (gfx) {
            command_list.SetGraphicsRootConstantBufferView(bind_point, buff->GetResource()->GetGPUVirtualAddress());
        }
        else {
            command_list.SetComputeRootConstantBufferView(bind_point, buff->GetResource()->GetGPUVirtualAddress());
        }
	}
}