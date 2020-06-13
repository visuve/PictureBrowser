#pragma once

class ImageCache
{
public:
	ImageCache() = default;
	~ImageCache();

	bool SetCurrent(const std::filesystem::path& path);
	Gdiplus::Image* Current();
	Gdiplus::Image* Get(const std::filesystem::path& path);

	void Clear();

private:
	Gdiplus::Image* Load(const std::filesystem::path& path);

	std::map<std::filesystem::path, std::unique_ptr<Gdiplus::Image>> m_cache;
	std::filesystem::path m_currentImage;
};