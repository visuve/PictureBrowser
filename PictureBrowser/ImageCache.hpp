#pragma once

namespace PictureBrowser
{
	class ImageCache
	{
	public:
		ImageCache(bool useCaching);
		~ImageCache();

		bool SetCurrent(const std::filesystem::path& path);
		std::shared_ptr<Gdiplus::Bitmap> Current();
		std::shared_ptr<Gdiplus::Bitmap> Get(const std::filesystem::path& path);

		void Clear();

	private:
		std::shared_ptr<Gdiplus::Bitmap> Load(const std::filesystem::path& path);

		std::map<std::filesystem::path, std::shared_ptr<Gdiplus::Bitmap>> _cache;
		std::filesystem::path _currentImage;
		bool _useCaching = true;
	};
}