#include "stdafx.h"
#include "TextureManager.h"

#include "Texture.h"

std::map<std::wstring, std::unique_ptr<Texture>> TextureManager::m_textures;

TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
}

void TextureManager::AddTexture(const std::wstring &name)
{
	if (m_textures.find(name) == m_textures.end())
	{
		m_textures.insert({ name , std::unique_ptr<Texture>(std::make_unique<Texture>(name)) });
	}
}

Texture* TextureManager::FindTexture(const std::wstring &name)
{
	Texture* text(nullptr);

	auto it = m_textures.find(name);
	if (it != m_textures.end())
		text = it->second.get();

	return text;
}

void TextureManager::Clear()
{
	for (auto &text : m_textures)
	{
		text.second->Clear();
	}
}
