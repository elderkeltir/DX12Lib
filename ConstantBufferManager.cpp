#include "ConstantBufferManager.h"
#include "GpuResource.h"
#include "DXHelper.h"

// #include "DXAppImplementation.h"

// extern DXAppImplementation *gD3DApp;


void ConstantBufferManager::OnInit() {
    {
        m_scene_cb.swap(std::make_unique<GpuResource>());
        m_scene_cb->CreateBuffer(HeapBuffer::BufferType::bt_upload, calc_cb_size(sizeof(SceneCB)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_GENERIC_READ, L"Scene_cb");
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.SizeInBytes = calc_cb_size(sizeof(SceneCB));
        m_scene_cb->Create_CBV(desc);
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            buff->Map();
        }
    }
}

void ConstantBufferManager::Destroy() {
    if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
        buff->Unmap();
    }
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix){
    if (id == Constants::cM){
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&model_cb->M, matrix);
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->V, matrix);
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4x4(&scene_cb->P, matrix);
        }
    }
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix){
    if (id == Constants::cM){
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()){
            ModelCB* model_cb = (ModelCB*) buff->GetCpuData();
            model_cb->M = matrix;
        }
    }
    else if (id == Constants::cV){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->V = matrix;
        }
    }
    else if (id == Constants::cP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->P = matrix;
        }
    }
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec){
        if (id == Constants::cCP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            DirectX::XMStoreFloat4(&scene_cb->CamPos, vec);
        }
    }
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec){
    if (id == Constants::cCP){
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()){
            SceneCB* scene_cb = (SceneCB*) buff->GetCpuData();
            scene_cb->CamPos = vec;
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

void ConstantBufferManager::CommitCB(ComPtr<ID3D12GraphicsCommandList6>& command_list, uint32_t id)
{
    if (id == 0) {
        if (std::shared_ptr<HeapBuffer> buff = m_model_cb->GetBuffer().lock()) {
            command_list->SetGraphicsRootConstantBufferView(bi_model_cb, buff->GetResource()->GetGPUVirtualAddress());
        }
    }
    else if (id == 1) {
        if (std::shared_ptr<HeapBuffer> buff = m_scene_cb->GetBuffer().lock()) {
            command_list->SetGraphicsRootConstantBufferView(bi_scene_cb, buff->GetResource()->GetGPUVirtualAddress());
        }
    }
}
