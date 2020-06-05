#pragma once

class ImageCache
{
public:
	ImageCache() = default;
	~ImageCache();

	Gdiplus::Image* Get(const std::filesystem::path& path);
	void Clear();


private:
	std::map<std::filesystem::path, std::unique_ptr<Gdiplus::Image>> m_cache;
};

