#include "Techniques.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "ShaderManager.h"

extern VkBackend* gBackend;

VkSampler CreateSampler(VkFilter mag, VkFilter min, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, VkBool32 anisotropy = VK_FALSE) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
    VkPhysicalDevice physical_device = gBackend->GetDevice()->GetPhysicalDevice();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = mag;
    samplerInfo.minFilter = min;
    samplerInfo.addressModeU = u;
    samplerInfo.addressModeV = v;
    samplerInfo.addressModeW = w;
    samplerInfo.anisotropyEnable = anisotropy;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties); // TODO: move to backend. get all the necessary props there and pass backend as 1st arg to every constructor

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    VkSampler sampler;
    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));

    return sampler;
}

VkDescriptorSetLayoutBinding CreateSetbinding(VkDescriptorType type, uint32_t binding, uint32_t count, VkShaderStageFlags shader_vis) {
	VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = binding;
    ubo_layout_binding.descriptorType = type;
    ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = shader_vis;
	ubo_layout_binding.pImmutableSamplers = nullptr;

    return ubo_layout_binding;
}

void AddSamplers(std::vector<VkDescriptorSetLayoutBinding> &layout_bindings) {
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLER, 0, 7, VK_SHADER_STAGE_FRAGMENT_BIT));
}

Techniques::RenderPass CreateRenderPass(const std::vector<VkFormat> &formats, bool depth) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    Techniques::RenderPass rpass;

    uint32_t size_attachments = (formats.size() + uint8_t(depth));

    std::vector<VkAttachmentDescription> attachments(size_attachments);
    uint32_t i = 0;
    for (; i < formats.size(); i++) {
        attachments[i].format = formats[i];
        attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (depth) {
        i++;
        attachments[i].format = VK_FORMAT_D32_SFLOAT;
        attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    std::vector<VkAttachmentReference> colorAttachments(formats.size());
    for (i = 0; i < formats.size(); i++) {
        colorAttachments[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorAttachments.size();
    subpass.pColorAttachments = colorAttachments.data();

    if (depth) {
        VkAttachmentReference depthAttachmentRef = { ++i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  };

        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }
    else {
        subpass.pDepthStencilAttachment = nullptr;
    }
    

    VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies = nullptr;

    VkRenderPass renderPass = 0;
    VK_CHECK(vkCreateRenderPass(device, &createInfo, 0, &renderPass));

    rpass.depth = depth;
    rpass.rpass = renderPass;

    return rpass;
}

void Techniques::CreateRootSignature_0(RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // desc set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // model cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 5, VK_SHADER_STAGE_FRAGMENT_BIT)); // gbuffer textures

    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));// scene cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));// materials cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 5, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));// vertex buffer // TODO: srv, cbv?
    AddSamplers(layout_bindings);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layout_bindings.size();
	layoutInfo.pBindings = layout_bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    root_sign->Init(layout_bindings, descriptorSetLayout);
}

void Techniques::CreateRootSignature_1(RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // desc set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 6, VK_SHADER_STAGE_FRAGMENT_BIT)); // gbuffer textures
    //layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // filler
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // model cb

    AddSamplers(layout_bindings);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layout_bindings.size();
	layoutInfo.pBindings = layout_bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    root_sign->Init(layout_bindings, descriptorSetLayout);
}

void Techniques::CreateRootSignature_2(RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // desc set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // model cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 6, VK_SHADER_STAGE_FRAGMENT_BIT)); // gbuffer textures
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // scene cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // light cb
    AddSamplers(layout_bindings);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layout_bindings.size();
	layoutInfo.pBindings = layout_bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    root_sign->Init(layout_bindings, descriptorSetLayout);
}

void Techniques::CreateRootSignature_3(RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // desc set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;


    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // model cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 5, VK_SHADER_STAGE_FRAGMENT_BIT)); // gbuffer textures
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // scene cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)); // light cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 5, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));// vertex buffer // TODO: srv, cbv?
    AddSamplers(layout_bindings);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layout_bindings.size();
	layoutInfo.pBindings = layout_bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    root_sign->Init(layout_bindings, descriptorSetLayout);
}

void Techniques::CreateRootSignature_4(RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // desc set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_COMPUTE_BIT)); // ssao cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 5, VK_SHADER_STAGE_COMPUTE_BIT)); // gbuffer textures
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, VK_SHADER_STAGE_COMPUTE_BIT)); // scene cb
    layout_bindings.push_back(CreateSetbinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 5, VK_SHADER_STAGE_COMPUTE_BIT)); // uav
    AddSamplers(layout_bindings);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layout_bindings.size();
	layoutInfo.pBindings = layout_bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    root_sign->Init(layout_bindings, descriptorSetLayout);
}

static Techniques::TechniqueVk CreateTechnique_0(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"g_buffer_vs.hlsl";
    tech.ps = L"g_buffer_ps.hlsl";
    tech.vertex_type = 0;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_1(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"g_buffer_vs.hlsl";
    tech.ps = L"g_buffer_ps.hlsl";
    tech.vertex_type = 0;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_2(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"quad_screen_vs.hlsl";
    tech.ps = L"post_processing_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_3(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"quad_screen_vs.hlsl";
    tech.ps = L"def_shading_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_4(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"skybox_vs.hlsl";
    tech.ps = L"skybox_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_5(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"skybox_vs.hlsl";
    tech.ps = L"skybox_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_compute);        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = cs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(cs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            shaderStageCreateInfo.module = shaderModule;
        }
    }

    shaderStageCreateInfo.pName = "main";
    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = layout;
    VkPipeline compute_pipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &compute_pipeline);

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = compute_pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_6(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"skybox_vs.hlsl";
    tech.ps = L"skybox_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_compute);        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = cs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(cs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            shaderStageCreateInfo.module = shaderModule;
        }
    }

    shaderStageCreateInfo.pName = "main";
    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = layout;
    VkPipeline compute_pipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &compute_pipeline);

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = compute_pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_7(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"terrain_vs.hlsl";
    tech.ps = L"terrain_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_8(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"water_vs.hlsl";
    tech.ps = L"water_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_9(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"shadow_vs.hlsl";
    tech.ps = L"";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // shaders
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = vs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[0].module = shaderModule;
        }
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = ps_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(ps_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            stages[1].module = shaderModule;
        }
    }

    createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

    // IA
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;
    
    // viewport
    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

    // raster
    VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

    // depth
    if (rp.depth) {
        VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
        createInfo.pDepthStencilState = &depthStencilState;
    }

    // blend
    VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

    // dynamic?
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = rp.rpass;

    // pipeline
    VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = pipeline;

    return tech;
}

static Techniques::TechniqueVk CreateTechnique_10(RootSignature& root_sign, const Techniques::RenderPass& rp, std::optional<std::wstring> dbg_name = std::nullopt){
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    // Tech
    Techniques::TechniqueVk tech;
    tech.vs = L"shadow_vs.hlsl";
    tech.ps = L"";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    // CreatePipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts = root_sign.GetLayout();
	pipelineLayoutInfo.pSetLayouts = &layouts;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

    // pipeline
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_compute);        
        {
            VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = cs_blob->size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(cs_blob->data.data());

            VkShaderModule shaderModule = 0;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

            shaderStageCreateInfo.module = shaderModule;
        }
    }

    shaderStageCreateInfo.pName = "main";
    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = layout;
    VkPipeline compute_pipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &compute_pipeline);

    tech.pipeline_layout = layout;
    tech.rp_id = rp.id;
    tech.pipeline = compute_pipeline;

    return tech;
}

void Techniques::OnInit(std::optional<std::wstring> dbg_name){
    auto device = gBackend->GetDevice()->GetNativeObject();
    
    // samplers
    m_samplers.push_back(CreateSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT));
    m_samplers.push_back(CreateSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
    m_samplers.push_back(CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT));
    m_samplers.push_back(CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
    m_samplers.push_back(CreateSampler(VK_FILTER_CUBIC_IMG, VK_FILTER_CUBIC_IMG, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_TRUE));
    m_samplers.push_back(CreateSampler(VK_FILTER_CUBIC_IMG, VK_FILTER_CUBIC_IMG, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_TRUE));
    
    // TODO: check other params: 0, 0, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    m_samplers.push_back(CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_TRUE));

    {
        uint32_t id = 0;
        id = m_root_signatures.push_back();
        CreateRootSignature_0(&m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
        CreateRootSignature_1(&m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
        CreateRootSignature_2(&m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
		CreateRootSignature_3(&m_root_signatures[id], dbg_name);
		m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
		CreateRootSignature_4(&m_root_signatures[id], dbg_name);
		m_root_signatures[id].SetRSId(id);
    }

    {
        uint32_t id = 0;
        std::vector<VkFormat> formats = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT };
        id = m_render_passes.push_back(CreateRenderPass(formats, true)); // gbuffer
        m_render_passes[id].id = id;
        formats = { VK_FORMAT_R8G8B8A8_UNORM };
        id = m_render_passes.push_back(CreateRenderPass(formats, false)); // pp
        m_render_passes[id].id = id;
        formats = { VK_FORMAT_R16G16B16A16_SFLOAT };
        id = m_render_passes.push_back(CreateRenderPass(formats, false)); // def_shading
        m_render_passes[id].id = id;
        formats.clear();
        id = m_render_passes.push_back(CreateRenderPass(formats, true));
        m_render_passes[id].id = id;
    }

    {
        uint32_t id = 0;
        id = m_techniques.push_back(CreateTechnique_0(m_root_signatures[0], m_render_passes[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_1(m_root_signatures[0], m_render_passes[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_2(m_root_signatures[1], m_render_passes[1], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_3(m_root_signatures[2], m_render_passes[2], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_4(m_root_signatures[0], m_render_passes[0], dbg_name));
        m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_5(m_root_signatures[4], m_render_passes[0], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_6(m_root_signatures[4], m_render_passes[0], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_7(m_root_signatures[3], m_render_passes[0], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_8(m_root_signatures[3], m_render_passes[3], dbg_name));
		m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_9(m_root_signatures[0], m_render_passes[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_10(m_root_signatures[4], m_render_passes[0], dbg_name));
        m_techniques[id].id = id;
    }
}

bool Techniques::TechHasColor(uint32_t tech_id) {
    if (tech_id == 0) {
        return true;
    }
    else {
        return false;
    }
}

void Techniques::RebuildShaders(std::optional<std::wstring> dbg_name)
{
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
        shader_mgr->ResetCache();
    }

    auto device = gBackend->GetDevice()->GetNativeObject();

    m_techniques[0] = CreateTechnique_0(m_root_signatures[0], m_render_passes[0], dbg_name);
    m_techniques[0].id = 0;
	m_techniques[1] = CreateTechnique_1(m_root_signatures[0], m_render_passes[0], dbg_name);
	m_techniques[1].id = 1;
	m_techniques[2] = CreateTechnique_2(m_root_signatures[1], m_render_passes[1], dbg_name);
	m_techniques[2].id = 2;
	m_techniques[3] = CreateTechnique_3(m_root_signatures[2], m_render_passes[2], dbg_name);
	m_techniques[3].id = 3;
	m_techniques[4] = CreateTechnique_4(m_root_signatures[0], m_render_passes[0], dbg_name);
	m_techniques[4].id = 4;
	m_techniques[5] = CreateTechnique_5(m_root_signatures[4], m_render_passes[0], dbg_name);
	m_techniques[5].id = 5;
	m_techniques[6] = CreateTechnique_6(m_root_signatures[4], m_render_passes[0], dbg_name);
	m_techniques[6].id = 6;
	m_techniques[7] = CreateTechnique_7(m_root_signatures[3], m_render_passes[0], dbg_name);
	m_techniques[7].id = 7;
	m_techniques[8] = CreateTechnique_8(m_root_signatures[3], m_render_passes[3], dbg_name);
	m_techniques[8].id = 8;
    m_techniques[9] = CreateTechnique_9(m_root_signatures[0], m_render_passes[0], dbg_name);
    m_techniques[9].id = 9;
    m_techniques[10] = CreateTechnique_10(m_root_signatures[4], m_render_passes[0], dbg_name);
    m_techniques[10].id = 10;
}
