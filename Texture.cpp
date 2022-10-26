#include "Texture.h"

#include <d3dx12.h>
#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>

Texture::Texture(std::wstring name) :
	m_name(std::move(name)),
	m_isLoaded(false)
{
}

Texture::~Texture()
{
}

ID3D12Resource* Texture::GetTextureRes() const
{
	return m_textureRes.Get();
}

const std::wstring& Texture::GetName() const
{
	return m_name;
}

bool Texture::IsLoaded() const
{
	return m_isLoaded;
}

bool Texture::Load(ID3D12Device2* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* commandQueue)
{
    DirectX::ResourceUploadBatch upload(device);

    upload.Begin();
    HRESULT res = DirectX::CreateDDSTextureFromFile(device, upload, m_name.c_str(), m_textureRes.ReleaseAndGetAddressOf());
	if (res == S_OK)
	{
        // Upload the resources to the GPU.
        auto finish = upload.End(commandQueue);

        // Wait for the upload thread to terminate
        finish.wait();

        m_isLoaded = true;
		return true;
	}
	else
	{
		//std::string name(m_name.begin(), m_name.end());
		// LogError("Failed to load texture from DDS: %s", name);

		return false;
	}
}

void Texture::Clear()
{
	m_textureRes = nullptr;
	m_textureResUpload = nullptr;
	m_isLoaded = false;
}