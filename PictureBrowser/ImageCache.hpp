#pragma once

namespace PictureBrowser
{
	class ImageCache
	{
	public:
		ImageCache(bool useCaching);
		~ImageCache();

		bool SetCurrent(const std::filesystem::path& path);
		Gdiplus::Bitmap* Current();
		Gdiplus::Bitmap* Get(const std::filesystem::path& path);

		void Clear();

	private:
		Gdiplus::Bitmap* Load(const std::filesystem::path& path);

		std::map<std::filesystem::path, std::unique_ptr<Gdiplus::Bitmap>> _cache;
		std::filesystem::path _currentImage;
		bool _useCaching = true;
	};
}