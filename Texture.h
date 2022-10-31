#pragma once

#include <directx/d3d12.h>
#include <wrl.h>
#include <string>

struct ID3D12Resource;
struct ID3D12Device2;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;

using Microsoft::WRL::ComPtr;

class Texture
{
public:
	Texture(std::wstring name);
	~Texture();
	ID3D12Resource* GetTextureRes() const;
	const std::wstring& GetName() const;
	bool IsLoaded() const;

	bool Load(ID3D12Device2* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* commandQueue);
	void Clear();
private:
	std::wstring m_name;
	bool m_isLoaded;
	ComPtr<ID3D12Resource> m_textureRes;
	ComPtr<ID3D12Resource> m_textureResUpload;
};
