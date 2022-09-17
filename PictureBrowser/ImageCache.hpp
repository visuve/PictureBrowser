#pragma once

namespace PictureBrowser
{
	class ImageCache
	{
	public:
		ImageCache(bool useCaching);
		~ImageCache();

		bool SetCurrent(const std::filesystem::path& path);
		Gdiplus::Image* Current();
		Gdiplus::Image* Get(const std::filesystem::path& path);

		void Clear();

	private:
		Gdiplus::Image* Load(const std::filesystem::path& path);

		std::map<std::filesystem::path, std::unique_ptr<Gdiplus::Image>> _cache;
		std::filesystem::path _currentImage;
		bool _useCaching = true;
	};
}