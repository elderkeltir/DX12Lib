#include "ShaderManager.h"
#include <assert.h>
#include "DxBackend.h"

#include <dxc/dxcapi.h>         // Be sure to link with dxcompiler.lib.
//#include <dxc/d3d12shader.h>    // Shader reflection.

#include <unordered_set>
#include <exception>

extern DxBackend* gBackend;

#define ThrowIfFailed(exp) exp // TODO: hack

const wchar_t* targets[] = {
	L"vs_6_5",
	L"ps_6_5",
	L"cs_6_5",
};

class CustomIncludeHandler : public IDxcIncludeHandler
{
public:
	HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
	{
		if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
			ComPtr<IDxcBlobEncoding> pEncoding;
			std::wstring file_name(pFilename);
			file_name.erase(0, 2);
			std::filesystem::path full_path = shader_mgr->GetShaderSourceDir() / file_name;
			if (IncludedFiles.find(full_path) != IncludedFiles.end())
			{
				// Return empty string blob if this file has been included before
				static const char nullStr[] = " ";
				shader_mgr->GetShaderCompilerUtils()->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, pEncoding.GetAddressOf());
				*ppIncludeSource = pEncoding.Detach();
				return S_OK;
			}

			HRESULT hr = shader_mgr->GetShaderCompilerUtils()->LoadFile(full_path.c_str(), nullptr, pEncoding.GetAddressOf());
			if (SUCCEEDED(hr))
			{
				IncludedFiles.insert(full_path);
				*ppIncludeSource = pEncoding.Detach();
			}
			else
			{
				*ppIncludeSource = nullptr;
			}
			return hr;
		}

		return E_FAIL;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
	{
		if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
			return shader_mgr->GetDefaultInclHandler()->QueryInterface(riid, ppvObject);
		}

		return E_FAIL;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
	ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

	std::unordered_set<std::wstring> IncludedFiles;
};

static CustomIncludeHandler* s_custom_handler = nullptr;

ShaderManager::ShaderManager()
{
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
	m_utils->CreateDefaultIncludeHandler(&m_includeHandler);

	s_custom_handler = new CustomIncludeHandler;

	m_shader_source_dir = gBackend->GetRootDir() / L"shaders";
	m_shader_bin_dir = gBackend->GetRootDir() / L"build" / L"src" / L"shaders";
}

ShaderManager::~ShaderManager()
{
	delete s_custom_handler;
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

	// check if shader already compiled
	if (std::filesystem::exists(full_path_bin)) {
		// check if modified of bin > moifided of hlsl
		if (std::filesystem::last_write_time(full_path_bin) > std::filesystem::last_write_time(full_path_hlsl)) {
			FILE* fp = NULL;
			_wfopen_s(&fp, full_path_bin.wstring().c_str(), L"rb");
			fseek(fp, 0, SEEK_END);
			uint32_t fsize = ftell(fp);
			fseek(fp, 0, SEEK_SET);  /* same as rewind(f); */
			std::vector<uint8_t> raw_data(fsize);
			fread(raw_data.data(), fsize, 1, fp);
			fclose(fp);

			ShaderBlob blob;
			blob.data = raw_data;
			blob.name = name;

			const uint32_t idx = m_loaded_shaders.push_back(blob);
			pShader = &m_loaded_shaders[idx];

			return pShader;
		}
	}

	//
	// COMMAND LINE:
	// dxc myshader.hlsl -E main -T ps_6_0 -Zi -D MYDEFINE=1 -Fo myshader.bin -Fd myshader.pdb -Qstrip_reflect
	//
	std::wstring incl_dir(L"-I ");
	incl_dir = incl_dir + m_shader_source_dir.c_str();
	LPCWSTR pszArgs[] =
	{
		name.c_str(),                   // Optional shader source file name for error reporting
		// and for PIX shader source view.  
L"-E", entry_point.c_str(),     // Entry point.
L"-T", targets[target],         // Target.
#ifdef _DEBUG
		L"-Od",                         // Disable optimization for better shader debugging
#else
		L"-O3",                         // Default optimization level in release
#endif // _DEBUG
		//L"-I ..\\content\\shaders",                  // include dirs
		incl_dir.c_str(),
		L"-Vi",                         // Display details about the include process
		L"-Zs",                         // Enable debug information (slim format)
		L"-Zpr",                        // Pack matrices in row-major order
		L"-WX",                         // Treat warnings as errors
		L"-D", L"MYDEFINE=1",           // A single define.
		L"-Fo", bin_name.c_str(),       // Optional. Stored in the pdb. 
		//L"-Fd", pdb_name.c_str(),       // The file name of the pdb. This must either be supplied
										// or the autogenerated file name must be used.
		//L"-Qstrip_reflect",             // Strip reflection into a separate blob. 
	};

	//
	// Open source file.  
	//

	ComPtr<IDxcBlobEncoding> pSource = nullptr;
	// TODO: check how it works on linux, is HRESULT a thing there?..
	ThrowIfFailed(m_utils->LoadFile(full_path_hlsl.wstring().c_str(), nullptr, &pSource));
	DxcBuffer Source;
	Source.Ptr = pSource->GetBufferPointer();
	Source.Size = pSource->GetBufferSize();
	Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

	//
	// Compile it with specified arguments.
	//
	ComPtr<IDxcResult> pResults;
	ThrowIfFailed(m_compiler->Compile(
		&Source,                // Source buffer.
		pszArgs,                // Array of pointers to arguments.
		_countof(pszArgs),      // Number of arguments.
		s_custom_handler,        // User-provided interface to handle #include directives (optional).
		IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
	));

	s_custom_handler->IncludedFiles.clear();

	//
	// Print errors if present.
	//
	ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	ThrowIfFailed(pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));
	// Note that d3dcompiler would return null if no errors or warnings are present.
	// IDxcCompiler3::Compile will always return an error buffer, but its length
	// will be zero if there are no warnings or errors.
	if (pErrors != nullptr && pErrors->GetStringLength() != 0) {
		auto errors = pErrors->GetStringPointer();
		wprintf(L"Warnings and Errors:\n%S\n", errors);
		assert(false);
	}

	//
	// Quit if the compilation failed.
	//
	HRESULT hrStatus;
	ThrowIfFailed(pResults->GetStatus(&hrStatus));
	if (FAILED(hrStatus))
	{
		wprintf(L"Compilation Failed\n");
		assert(false);
		return pShader;
	}

	//
	// Save shader binary.
	//
	{
		ComPtr<IDxcBlob>pShaderDx = nullptr;
		ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
		ThrowIfFailed(pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShaderDx), &pShaderName));
		if (pShaderDx != nullptr)
		{
			FILE* fp = NULL;
			std::filesystem::create_directory(m_shader_bin_dir);
			const std::wstring path_to_shader_bin((m_shader_bin_dir / pShaderName->GetStringPointer()).wstring());
			_wfopen_s(&fp, path_to_shader_bin.c_str(), L"wb");
			fwrite(pShaderDx->GetBufferPointer(), pShaderDx->GetBufferSize(), 1, fp);
			fclose(fp);

			ShaderBlob blob;
			blob.data.resize(pShaderDx->GetBufferSize());
			memcpy_s(blob.data.data(), pShaderDx->GetBufferSize(), pShaderDx->GetBufferPointer(), pShaderDx->GetBufferSize());
			blob.name = name;

			const uint32_t idx = m_loaded_shaders.push_back(blob);
			pShader = &m_loaded_shaders[idx];
		}
	}

	//
	// Save pdb.
	//
	ComPtr<IDxcBlob> pPDB = nullptr;
	ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
	ThrowIfFailed(pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName));
	{
		FILE* fp = NULL;

		// Note that if you don't specify -Fd, a pdb name will be automatically generated.
		// Use this file name to save the pdb so that PIX can find it quickly.
		const std::wstring path_to_shader_pdb((m_shader_bin_dir / pPDBName->GetStringPointer()).wstring());
		_wfopen_s(&fp, path_to_shader_pdb.c_str(), L"wb");
		fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
		fclose(fp);
	}

	// //
	// // Print hash.
	// //
	// ComPtr<IDxcBlob> pHash = nullptr;
	// ThrowIfFailed(pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr));
	// if (pHash != nullptr)
	// {
	//     wprintf(L"Hash: ");
	//     DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
	//     for (int i = 0; i < _countof(pHashBuf->HashDigest); i++){
	//         wprintf(L"%.2x", pHashBuf->HashDigest[i]);
	//     }
	//     wprintf(L"\n");
	// }

	// //
	// // Demonstrate getting the hash from the PDB blob using the IDxcUtils::GetPDBContents API
	// //
	// ComPtr<IDxcBlob> pHashDigestBlob = nullptr;
	// ComPtr<IDxcBlob> pDebugDxilContainer = nullptr;
	// if (SUCCEEDED(m_utils->GetPDBContents(pPDB.Get(), &pHashDigestBlob, &pDebugDxilContainer)))
	// {
	//     // This API returns the raw hash digest, rather than a DxcShaderHash structure.
	//     // This will be the same as the DxcShaderHash::HashDigest returned from
	//     // IDxcResult::GetOutput(DXC_OUT_SHADER_HASH, ...).
	//     wprintf(L"Hash from PDB: ");
	//     const BYTE *pHashDigest = (const BYTE*)pHashDigestBlob->GetBufferPointer();
	//     assert(pHashDigestBlob->GetBufferSize() == 16); // hash digest is always 16 bytes.
	//     for (int i = 0; i < pHashDigestBlob->GetBufferSize(); i++){
	//         wprintf(L"%.2x", pHashDigest[i]);
	//     }
	//     wprintf(L"\n");

	//     // The pDebugDxilContainer blob will contain a DxilContainer formatted
	//     // binary, but with different parts than the pShader blob retrieved
	//     // earlier.
	//     // The parts in this container will vary depending on debug options and
	//     // the compiler version.
	//     // This blob is not meant to be directly interpreted by an application.
	// }

	// //
	// // Get separate reflection.
	// //
	// ComPtr<IDxcBlob> pReflectionData;
	// ThrowIfFailed(pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr));
	// if (pReflectionData != nullptr)
	// {
	//     // Optionally, save reflection blob for later here.

	//     // Create reflection interface.
	//     DxcBuffer ReflectionData;
	//     ReflectionData.Encoding = DXC_CP_ACP;
	//     ReflectionData.Ptr = pReflectionData->GetBufferPointer();
	//     ReflectionData.Size = pReflectionData->GetBufferSize();

	//     ComPtr <ID3D12ShaderReflection> pReflection;
	//     m_utils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection));

	//     // Use reflection interface here.

	// }

	return pShader;
}

const std::filesystem::path& ShaderManager::GetShaderSourceDir() const {
	return m_shader_source_dir;
}

const std::filesystem::path& ShaderManager::GetShaderBinaryDir() const {
	return m_shader_bin_dir;
}