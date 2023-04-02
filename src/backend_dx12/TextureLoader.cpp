#include "TextureLoader.h"
#include "dx12_helper.h"
#include "defines.h"
#include "DxBackend.h"
#include "DxDevice.h"
#include "IGpuResource.h"
#include "ICommandList.h"

extern DxBackend* gBackend;

ITextureLoader* CreateTextureLoader(const std::filesystem::path &root_dir) {
	return new TextureLoader(root_dir);
}

TextureLoader::TextureLoader(const std::filesystem::path& root_dir)
{
	m_texture_dir = root_dir / L"content" / L"textures";
}

void TextureLoader::OnInit()
{
	

	// win32 specific for texture loader lib
	// TODO: remove damn dx-textures?
	// or move into backend?
	ThrowIfFailed(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));
}

ITextureLoader::TextureData* TextureLoader::LoadTextureOnCPU(const std::wstring& name)
{
	std::filesystem::path related_path(name);
	const std::wstring filename = related_path.filename().wstring();

	const auto it = std::find_if(m_load_textures.begin(), m_load_textures.end(), [&filename](TextureDataDx& texture) { return (texture.name == filename); });
	if (it != m_load_textures.end()) {
		return &(*it);
	}
	else {
		TextureDataDx* texture = &m_load_textures[m_load_textures.push_back()];
		texture->name = filename;
		std::filesystem::path full_path((m_texture_dir / filename));
		assert(std::filesystem::exists(full_path));

		if (full_path.extension() == ".dds" || full_path.extension() == ".DDS") {
			ThrowIfFailed(DirectX::LoadFromDDSFile(full_path.wstring().c_str(), DirectX::DDS_FLAGS_NONE, &texture->meta_data, texture->scratch_image));
		}
		else if (full_path.extension() == ".hdr" || full_path.extension() == ".HDR") {
			ThrowIfFailed(DirectX::LoadFromHDRFile(full_path.wstring().c_str(), &texture->meta_data, texture->scratch_image));
		}
		else if (full_path.extension() == ".tga" || full_path.extension() == ".TGA") {
			ThrowIfFailed(DirectX::LoadFromTGAFile(full_path.wstring().c_str(), &texture->meta_data, texture->scratch_image));
		}
		else {
			ThrowIfFailed(DirectX::LoadFromWICFile(full_path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, &texture->meta_data, texture->scratch_image));
		}

		if (texture->meta_data.format != DXGI_FORMAT_R8_UNORM) {
			texture->meta_data.format = DirectX::MakeSRGB(texture->meta_data.format);
		}

		return (TextureData*)texture;
	}
}

void TextureLoader::LoadTextureOnGPU(ICommandList* command_list, IGpuResource* res, ITextureLoader::TextureData* tex_data)
{
	TextureLoader::TextureDataDx* tex_data_dx = (TextureLoader::TextureDataDx*)tex_data;
	SRVdesc::SRVdimensionType srv_dim = SRVdesc::SRVdimensionType::srv_dt_texture2d;
	ResourceDesc tex_desc;

	switch (tex_data_dx->meta_data.dimension)
	{
	case DirectX::TEX_DIMENSION_TEXTURE1D:
		tex_desc = ResourceDesc::tex_1d((ResourceFormat)tex_data_dx->meta_data.format, static_cast<UINT64>(tex_data_dx->meta_data.width), static_cast<UINT16>(tex_data_dx->meta_data.arraySize));
		srv_dim = SRVdesc::SRVdimensionType::srv_dt_texture1d;
		break;
	case DirectX::TEX_DIMENSION_TEXTURE2D:
		tex_desc = ResourceDesc::tex_2d((ResourceFormat)tex_data_dx->meta_data.format, static_cast<UINT64>(tex_data_dx->meta_data.width), static_cast<UINT>(tex_data_dx->meta_data.height), static_cast<UINT16>(tex_data_dx->meta_data.arraySize));
		srv_dim = (tex_data_dx->meta_data.arraySize > 1) ? SRVdesc::SRVdimensionType::srv_dt_texturecube : SRVdesc::SRVdimensionType::srv_dt_texture2d;
		break;
	case DirectX::TEX_DIMENSION_TEXTURE3D:
		srv_dim = SRVdesc::SRVdimensionType::srv_dt_texture3d;
		tex_desc = ResourceDesc::tex_3d((ResourceFormat)tex_data_dx->meta_data.format, static_cast<UINT64>(tex_data_dx->meta_data.width), static_cast<UINT>(tex_data_dx->meta_data.height), static_cast<UINT16>(tex_data_dx->meta_data.depth));
		break;
	default:
		throw std::exception("Invalid texture dimension.");
		break;
	}

	res->CreateTexture(HeapType::ht_default, tex_desc, ResourceState::rs_resource_state_copy_dest, nullptr, std::wstring(tex_data_dx->name).append(L"model_srv_").c_str());

	const uint32_t image_count = (uint32_t)tex_data_dx->scratch_image.GetImageCount();
	std::vector<SubresourceData> subresources(image_count);
	const DirectX::Image* pImages = tex_data_dx->scratch_image.GetImages();
	for (uint32_t i = 0; i < (uint32_t)image_count; ++i) {
		auto& subresource = subresources[i];
		subresource.row_pitch = pImages[i].rowPitch;
		subresource.slice_pitch = pImages[i].slicePitch;
		subresource.data = pImages[i].pixels;
	}
	res->LoadBuffer(command_list, 0, (uint32_t)subresources.size(), subresources.data());
	command_list->ResourceBarrier(*res, ResourceState::rs_resource_state_pixel_shader_resource);

	SRVdesc srv_desc = {};

	srv_desc.format = (ResourceFormat)tex_data_dx->meta_data.format; // TODO: remove when load text moved etc.
	srv_desc.dimension = srv_dim;
	if (srv_dim == SRVdesc::SRVdimensionType::srv_dt_texture2d) {
		srv_desc.texture2d.most_detailed_mip = 0;
		srv_desc.texture2d.mip_levels = (UINT)tex_data_dx->meta_data.mipLevels;
		srv_desc.texture2d.res_min_lod_clamp = 0.0f;
	}
	else if (srv_dim == SRVdesc::SRVdimensionType::srv_dt_texturecube) {
		srv_desc.texture_cube.most_detailed_mip = 0;
		srv_desc.texture_cube.mip_levels = (UINT)tex_data_dx->meta_data.mipLevels;
		srv_desc.texture_cube.res_min_lod_clamp = 0.0f;
	}

	res->Create_SRV(srv_desc);
}
