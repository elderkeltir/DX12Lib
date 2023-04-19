#include "TextureLoader.h"
#include "vk_helper.h"
#include "defines.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "IGpuResource.h"
#include "ICommandList.h"
#include "HeapBuffer.h"
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // TODO: move loading image to another class. probably after vfs implementation?

extern VkBackend* gBackend;

ITextureLoader* CreateTextureLoader(const std::filesystem::path &root_dir) {
	return new TextureLoader(root_dir);
}

TextureLoader::TextureLoader(const std::filesystem::path& root_dir)
{
	m_texture_dir = root_dir / L"content" / L"textures";
}

void TextureLoader::OnInit()
{
	
}

ITextureLoader::TextureData* TextureLoader::LoadTextureOnCPU(const std::wstring& name)
{
	std::filesystem::path related_path(name);
	const std::wstring filename = related_path.filename().wstring();

	const auto it = std::find_if(m_load_textures.begin(), m_load_textures.end(), [&filename](TextureDataVk& texture) { return (texture.name == filename); });
	if (it != m_load_textures.end()) {
		return &(*it);
	}
	else {
		
		TextureDataVk* texture = &m_load_textures[m_load_textures.push_back()];
		texture->name = filename;
		std::filesystem::path full_path((m_texture_dir / filename));

		if (full_path.extension() == ".dds" || full_path.extension() == ".DDS") {
			assert(false); // TODO: write own DDS loader later - https://www.reddit.com/r/vulkan/comments/85yo3d/image_libraries_for_vulkan_projects/
		}
		else {
			assert(std::filesystem::exists(full_path));
			
			std::string file_path_full = full_path.string();

			int w, h, comp;
			assert(stbi_info(file_path_full.c_str(), &w, &h, &comp));
			const bool is_16_bits_per_channel = stbi_is_16_bit(file_path_full.c_str());
			stbi_uc* pixels = stbi_load(file_path_full.c_str(), &w, &h, &comp, STBI_rgb_alpha);
			assert(pixels);
			const uint32_t bytes_pp = (is_16_bits_per_channel ? 8 : 4);
			
			texture->size = w * h * bytes_pp;
			texture->channels = comp;
			texture->height = h;
			texture->width = w;
			texture->pixels = pixels;
			texture->clear_cb = stbi_image_free;
		}

		return (TextureData*)texture;
	}
}

void TextureLoader::LoadTextureOnGPU(ICommandList* command_list, IGpuResource* res, ITextureLoader::TextureData* tex_data)
{
	TextureLoader::TextureDataVk* tex_data_vk = (TextureLoader::TextureDataVk*)tex_data;

	ResourceFormat format = ResourceFormat::rf_unknown;
	if (tex_data_vk->channels == 4 && (tex_data_vk->size / (tex_data_vk->width * tex_data_vk->height)) == 4){
		format = ResourceFormat::rf_b8g8r8a8_unorm_srgb;
	}
	else {
		assert(false); // TODO: add when needed
	}

	ResourceDesc tex_desc;
	tex_desc.format = format;
	tex_desc.width = tex_data_vk->width;
	tex_desc.height = tex_data_vk->height;
	HeapType h_type = HeapType(HeapType::ht_default | HeapType::ht_image_sampled | HeapType::ht_aspect_color_bit);
	res->CreateTexture(h_type, tex_desc, ResourceState::rs_resource_state_copy_dest, nullptr, std::wstring(tex_data->name).append(L"model_srv_").c_str());
	SubresourceData sub_res;
	sub_res.width = tex_data_vk->width;
	sub_res.height = tex_data_vk->height;
	sub_res.data = tex_data_vk->pixels;
	sub_res.slice_pitch = tex_data_vk->size;
	res->LoadBuffer(command_list, 0, 1, &sub_res);
	command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_pixel_shader_resource);
	
	SRVdesc des;
	res->Create_SRV(des);
}
