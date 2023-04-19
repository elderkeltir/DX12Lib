#include "ShaderManager.h"
#include <assert.h>
#include "VkBackend.h"

#include <cstdlib>
#include <sstream>

extern VkBackend* gBackend;

const char* targets[] = {
	"vs_6_5",
	"ps_6_5",
	"cs_6_5",
};

ShaderManager::ShaderManager()
{
	m_shader_source_dir = gBackend->GetRootDir() / L"shaders";
	m_shader_bin_dir = gBackend->GetRootDir() / L"build" / L"src" / L"shaders";
}

ShaderManager::~ShaderManager()
{
}

ShaderManager::ShaderBlob* ShaderManager::Load(const std::wstring& name, const std::wstring& entry_point, ShaderType target) {
	ShaderManager::ShaderBlob* pShader = nullptr;

	std::wstring pdb_name = name;
	pdb_name.erase(pdb_name.end() - 5, pdb_name.end());
	std::wstring bin_name = pdb_name;
	pdb_name += L".pdb";
	bin_name += L".bin";
	std::filesystem::path full_path_hlsl = m_shader_source_dir / name;
	std::filesystem::path full_path_bin = m_shader_bin_dir / bin_name;

	assert(std::filesystem::exists(full_path_hlsl));

	// check if shader already loaded in cache
	if (ShaderManager::ShaderBlob* cached_shader = GetShaderBLOB(name)) {
		return cached_shader;
	}

	auto load_shader_bin = [&] () {
		FILE* fp = fopen(full_path_bin.string().c_str(), "rb");
		assert(fp);
		fseek(fp, 0, SEEK_END);
		uint32_t fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);  /* same as rewind(f); */
		std::vector<uint8_t> raw_data(fsize);
		fread(raw_data.data(), fsize, 1, fp);
		fclose(fp);

		ShaderBlob blob;
		blob.data = raw_data;
		blob.name = name;
		blob.size = fsize;

		const uint32_t idx = m_loaded_shaders.push_back(blob);
		return &m_loaded_shaders[idx];
	};

	// check if shader already compiled
	if (std::filesystem::exists(full_path_bin)) {
		// check if modified of bin > moifided of hlsl
		if (std::filesystem::last_write_time(full_path_bin) > std::filesystem::last_write_time(full_path_hlsl)) {
			return load_shader_bin();
		}
	}

	// Re-Compile
	std::string dxc_fullpath = (m_shader_source_dir.parent_path().parent_path() / "DirectXShaderCompiler" / "build" / "bin" / "dxc").string();
	std::stringstream command;
	command << dxc_fullpath << " -T " << targets[target] << " -E main " << full_path_hlsl.string() << " -Fo " << full_path_bin.string() << " -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout";
	assert(!std::system(command.str().c_str()));
	
	return load_shader_bin();
}

const std::filesystem::path& ShaderManager::GetShaderSourceDir() const {
	return m_shader_source_dir;
}

const std::filesystem::path& ShaderManager::GetShaderBinaryDir() const {
	return m_shader_bin_dir;
}