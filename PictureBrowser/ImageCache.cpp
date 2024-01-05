#include "PCH.hpp"
#include "ImageCache.hpp"
#include "GdiExtensions.hpp"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	void CorrectRotationIfNeeded(Gdiplus::Image* image)
	{
		const Gdiplus::RotateFlipType rotation = GdiExtensions::GetRotation(image);

		if (rotation != Gdiplus::RotateFlipType::RotateNoneFlipNone)
		{
			image->RotateFlip(rotation);
		}
	}

	ImageCache::ImageCache(bool useCaching) :
		_useCaching(useCaching)
	{
	}

	ImageCache::~ImageCache()
	{
		Clear();
	}

	bool ImageCache::SetCurrent(const std::filesystem::path& path)
	{
		if (!Get(path))
		{
			return false;
		}

		_currentImage = path;
		return true;
	}

	std::shared_ptr<Gdiplus::Bitmap> ImageCache::Current()
	{
		return Get(_currentImage);
	}

	std::shared_ptr<Gdiplus::Bitmap> ImageCache::Get(const std::filesystem::path& path)
	{
		if (_useCaching)
		{
			const auto iter = _cache.find(path);

			if (iter != _cache.cend())
			{
				return iter->second;
			}

			LOGD << L"Not cached: " << path;
		}

		return Load(path);
	}

	void ImageCache::Clear()
	{
		_cache.clear();
	}

	std::shared_ptr<Gdiplus::Bitmap> ImageCache::Load(const std::filesystem::path& path)
	{
		Gdiplus::Image image(path.c_str());

		if (image.GetLastStatus() != Gdiplus::Status::Ok)
		{
			return nullptr;
		}

		CorrectRotationIfNeeded(&image);

		UINT width = image.GetWidth();
		UINT height = image.GetHeight();

		auto buffer = std::make_shared<Gdiplus::Bitmap>(width, height, PixelFormat24bppRGB);

		Gdiplus::Graphics graphics(buffer.get());
		graphics.DrawImage(&image, 0, 0, width, height);

		if (_useCaching)
		{
			_cache.emplace(path, buffer);
			LOGD << L"Cached: " << path;
		}

		return buffer;
	}
}