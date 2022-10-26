#pragma once

#include <map>
#include <memory>
#include <string>

class Texture;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();
	static void AddTexture(const std::wstring &name);
	static Texture* FindTexture(const std::wstring &name);

	static void Clear();

private:
	static std::map<std::wstring, std::unique_ptr<Texture>> m_textures;
};

