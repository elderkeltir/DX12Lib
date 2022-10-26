#pragma once

#include <map>
#include <memory>
#include <string>
#include <filesystem>

class Texture;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();
	void AddTexture(const std::wstring &name);
	Texture* FindTexture(const std::wstring &name);

	void Clear();

	const std::filesystem::path& GetTextureDir() const;

private:
	std::map<std::wstring, std::unique_ptr<Texture>> m_textures;

	std::filesystem::path m_texture_dir;
};

