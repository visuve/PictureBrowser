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

	Gdiplus::Bitmap* ImageCache::Current()
	{
		return Get(_currentImage);
	}

	Gdiplus::Bitmap* ImageCache::Get(const std::filesystem::path& path)
	{
		if (_useCaching)
		{
			const auto iter = _cache.find(path);

			if (iter != _cache.cend())
			{
				return iter->second.get();
			}

			LOGD << L"Not cached: " << path;
		}

		return Load(path);
	}

	void ImageCache::Clear()
	{
		_cache.clear();
	}

	Gdiplus::Bitmap* ImageCache::Load(const std::filesystem::path& path)
	{
		Gdiplus::Image* image = Gdiplus::Image::FromFile(path.c_str());

		if (!image)
		{
			return nullptr;
		}

		CorrectRotationIfNeeded(image);

		UINT width = image->GetWidth();
		UINT height = image->GetHeight();
		Gdiplus::PixelFormat format = image->GetPixelFormat();

		if ((format & PixelFormat8bppIndexed) == PixelFormat8bppIndexed)
		{
			// Somehow images with 8 bit depth are not drawn correclty otherwise...
			format = PixelFormat24bppRGB;
		}

		Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, format);
		Gdiplus::Graphics graphics(bitmap);
		graphics.DrawImage(image, 0, 0, width, height);

		if (_useCaching)
		{
			_cache.emplace(path, bitmap);
			LOGD << L"Cached: " << path;
		}

		delete image;
		return bitmap;
	}
}