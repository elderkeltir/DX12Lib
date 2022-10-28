#include "TextureManager.h"

#include "Texture.h"
#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

TextureManager::TextureManager()
{
	m_texture_dir = gD3DApp->GetRootDir() / L"content" / L"textures";
}


TextureManager::~TextureManager()
{
}

void TextureManager::AddTexture(const std::wstring &name)
{
	if (m_textures.find(name) == m_textures.end())
	{
		m_textures.insert({ name , std::make_shared<Texture>(name) });
	}
}

std::weak_ptr<Texture> TextureManager::FindTexture(const std::wstring &name)
{
	std::weak_ptr<Texture> text;

	auto it = m_textures.find(name);
	if (it != m_textures.end())
		text = it->second;

	return text;
}

void TextureManager::Clear()
{
	for (auto &text : m_textures)
	{
		text.second->Clear();
	}
}

const std::filesystem::path& TextureManager::GetTextureDir() const{
	return m_texture_dir;
}
