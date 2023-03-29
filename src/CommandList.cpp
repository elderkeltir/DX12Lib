#include "CommandList.h"
#include "defines.h"
#include "GpuResource.h"
#include "DXAppImplementation.h"
#include "ResourceDescriptor.h"
#include "DynamicGpuHeap.h"

extern DXAppImplementation* gD3DApp;

void CommandList::Reset()
{
    m_pso = uint32_t(-1);
    m_root_sign = uint32_t(-1);
}

void CommandList::RSSetViewports(uint32_t num_viewports, const ViewPort* viewports)
{
	m_command_list->RSSetViewports(num_viewports, (D3D12_VIEWPORT*)viewports); // TODO: later maybe create cast func
}

void CommandList::RSSetScissorRects(uint32_t num_rects, const RectScissors* rects)
{
	m_command_list->RSSetScissorRects(num_rects, (D3D12_RECT*)rects);
}

void CommandList::SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor)
{
    D3D12_GPU_DESCRIPTOR_HANDLE hndl;
    hndl.ptr = base_descriptor.ptr;
	m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, hndl);
}

void CommandList::SetComputeRootDescriptorTable(uint32_t root_parameter_index, GPUdescriptor base_descriptor)
{
    D3D12_GPU_DESCRIPTOR_HANDLE hndl;
    hndl.ptr = base_descriptor.ptr;
	m_command_list->SetComputeRootDescriptorTable(root_parameter_index, hndl);
}

void CommandList::SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<HeapBuffer>& buff)
{
	m_command_list->SetGraphicsRootConstantBufferView(root_parameter_index, buff->GetResource()->GetGPUVirtualAddress());
}

void CommandList::SetComputeRootConstantBufferView(uint32_t root_parameter_index, const std::shared_ptr<HeapBuffer>& buff)
{
	m_command_list->SetComputeRootConstantBufferView(root_parameter_index, buff->GetResource()->GetGPUVirtualAddress());
}

void CommandList::SetIndexBuffer(const IndexVufferView* view)
{
	D3D12_INDEX_BUFFER_VIEW view_native;
	view_native.BufferLocation = view->buffer_location;
	view_native.Format = (DXGI_FORMAT)view->format;
	view_native.SizeInBytes = view->size_in_bytes;

	m_command_list->IASetIndexBuffer(&view_native);
}

void CommandList::SetPrimitiveTopology(PrimitiveTopology primirive_topology)
{
	m_command_list->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)primirive_topology);
}

void CommandList::DrawInstanced(uint32_t vertex_per_instance, uint32_t instance_count, uint32_t start_vertex_location, uint32_t start_instance_location)
{
	m_command_list->DrawInstanced(vertex_per_instance, instance_count, start_vertex_location, start_instance_location);
}

void CommandList::DrawIndexedInstanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t start_index_location, int32_t base_vertex_location, uint32_t start_instance_location)
{
	m_command_list->DrawIndexedInstanced(index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location);
}

void CommandList::SetDescriptorHeap(const DynamicGpuHeap& dynamic_heap)
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { dynamic_heap.GetVisibleHeap().Get() };
	m_command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

void CommandList::SetRenderTargets(const std::vector<GpuResource*>& resources, GpuResource* depth_stencil_descriptor)
{
    const uint32_t rts_num = (uint32_t)resources.size();
    D3D12_CPU_DESCRIPTOR_HANDLE dvs;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, MAX_RTS_NUM> rtvs;
    for (uint32_t idx = 0; idx < rts_num; idx++) {
        if (std::shared_ptr<ResourceDescriptor> rtv = resources[idx]->GetRTV().lock()) {
            rtvs[idx].ptr = rtv->GetCPUhandle().ptr;
        }
    }

    if (depth_stencil_descriptor) {
        if (std::shared_ptr<ResourceDescriptor> depth_view = depth_stencil_descriptor->GetDSV().lock()) {
            dvs.ptr = depth_view->GetCPUhandle().ptr;
        }
    }

	m_command_list->OMSetRenderTargets(rts_num, rtvs.data(), false, depth_stencil_descriptor ? &dvs : nullptr);
}

void CommandList::ClearRenderTargetView(GpuResource* res, const float color[4], uint32_t num_rects, const RectScissors* rect)
{
    if (std::shared_ptr<ResourceDescriptor> render_target_view = res->GetRTV().lock()) {
        D3D12_CPU_DESCRIPTOR_HANDLE hndl;
        hndl.ptr = render_target_view->GetCPUhandle().ptr;
	    m_command_list->ClearRenderTargetView(hndl, color, num_rects, (D3D12_RECT*)rect);
    }
}

void CommandList::ClearDepthStencilView(GpuResource* res, ClearFlagsDsv clear_flags, float depth, uint8_t stencil, uint32_t num_rects, const RectScissors* rects)
{
    if (std::shared_ptr<ResourceDescriptor> depth_view = res->GetDSV().lock()) {
        D3D12_CPU_DESCRIPTOR_HANDLE hndl;
        hndl.ptr = depth_view->GetCPUhandle().ptr;
        m_command_list->ClearDepthStencilView(hndl, (D3D12_CLEAR_FLAGS)clear_flags, depth, stencil, num_rects, (D3D12_RECT*)rects);
    }
}

void CommandList::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z)
{
	m_command_list->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
}

void CommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, const std::shared_ptr<HeapBuffer>& buff)
{
	m_command_list->SetGraphicsRootShaderResourceView(root_parameter_index, buff->GetResource()->GetGPUVirtualAddress());
}

void CommandList::ResourceBarrier(std::shared_ptr<GpuResource>& res, uint32_t to) {
    if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()) {
        D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)res->GetState();
        D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
        if (calculated_from != to_native) {
            m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
            res->UpdateState((ResourceState)to);
        }
    }
}

void CommandList::ResourceBarrier(GpuResource& res, uint32_t to) {
    if (std::shared_ptr<HeapBuffer> buff = res.GetBuffer().lock()) {
        D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)res.GetState();
        D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
        if (calculated_from != to_native) {
            m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
            res.UpdateState((ResourceState)to);
        }
    }
}

void CommandList::ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, uint32_t to) {
    std::vector<CD3DX12_RESOURCE_BARRIER> resources;
    resources.reserve(res.size());
    for (auto gpu_res : res) {
        if (std::shared_ptr<HeapBuffer> buff = gpu_res->GetBuffer().lock()) {
            D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)gpu_res->GetState();
            D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
            if (calculated_from != to_native) {
                resources.push_back(CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
                gpu_res->UpdateState((ResourceState)to);
            }
        }
    }

    m_command_list->ResourceBarrier((uint32_t)resources.size(), resources.data());
}

void CommandList::SetPSO(uint32_t id) {
    m_pso = id;
    auto tech = gD3DApp->GetTechniqueById(id);

    m_command_list->SetPipelineState(tech->pipeline_state.Get());
}

void CommandList::SetRootSign(uint32_t id, bool gfx) {
    m_root_sign = id;

    auto root_sign = gD3DApp->GetRootSignById(id);
    auto& r = root_sign->GetRootSignature();
    if (m_type == CommandListType::clt_direct && gfx) {
        m_command_list->SetGraphicsRootSignature(r.Get());
    }
    else {
        m_command_list->SetComputeRootSignature(r.Get());
    }
}