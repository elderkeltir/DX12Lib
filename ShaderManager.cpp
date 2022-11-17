#include "ShaderManager.h"
#include <assert.h>
#include "DXAppImplementation.h"
#include "DXHelper.h"

#include <unordered_set>

extern DXAppImplementation *gD3DApp;

const wchar_t *targets[] = {
    L"vs_6_0",
    L"ps_6_0",
    L"cs_6_0",
};

ShaderManager::ShaderManager()
{
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
    m_utils->CreateDefaultIncludeHandler(&m_includeHandler);

    m_shader_source_dir = gD3DApp->GetRootDir() / L"content" / L"shaders";
    m_shader_bin_dir = gD3DApp->GetRootDir() / L"build" / L"shaders";
}

ShaderManager::ShaderBlob* ShaderManager::Load(const std::wstring &name, const std::wstring &entry_point, ShaderType target){
    ShaderManager::ShaderBlob* pShader = nullptr;

    std::wstring pdb_name = name;
    pdb_name.erase(pdb_name.end() -5, pdb_name.end());
    std::wstring bin_name = pdb_name;
    pdb_name += L".pdb";
    bin_name += L".bin";
    std::filesystem::path full_path_hlsl = m_shader_source_dir / name;
    std::filesystem::path full_path_bin = m_shader_bin_dir / bin_name;

    assert(std::filesystem::exists(full_path_hlsl));

    // check if shader already loaded in cache
    if (ShaderManager::ShaderBlob* cached_shader = GetShaderBLOB(name)){
        return cached_shader;
    }

    // check if shader already compiled
    if (std::filesystem::exists(full_path_bin)){
        // check if modified of bin > moifided of hlsl
        if (std::filesystem::last_write_time(full_path_bin) > std::filesystem::last_write_time(full_path_hlsl)){
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
        L"-I ..\\content\\shaders",                  // include dirs
        L"-Vi",                         // Display details about the include process
        L"-Zs",                         // Enable debug information (slim format)
        L"-D", L"MYDEFINE=1",           // A single define.
        L"-Fo", bin_name.c_str(),       // Optional. Stored in the pdb. 
        L"-Fd", pdb_name.c_str(),       // The file name of the pdb. This must either be supplied
                                        // or the autogenerated file name must be used.
        L"-Qstrip_reflect",             // Strip reflection into a separate blob. 
    };

    //
    // Open source file.  
    //

    ComPtr<IDxcBlobEncoding> pSource = nullptr;
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
        m_includeHandler.Get(),        // User-provided interface to handle #include directives (optional).
        IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
    ));

    //
    // Print errors if present.
    //
    ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    ThrowIfFailed(pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));
    // Note that d3dcompiler would return null if no errors or warnings are present.
    // IDxcCompiler3::Compile will always return an error buffer, but its length
    // will be zero if there are no warnings or errors.
    if (pErrors != nullptr && pErrors->GetStringLength() != 0){
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

const std::filesystem::path& ShaderManager::GetShaderSourceDir() const{
    return m_shader_source_dir;
}

const std::filesystem::path& ShaderManager::GetShaderBinaryDir() const{
    return m_shader_bin_dir;
}