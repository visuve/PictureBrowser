#include "PCH.hpp"
#include "ImageCache.hpp"
#include "GdiExtensions.hpp"
#include "LogWrap.hpp"

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

Gdiplus::Image* ImageCache::Current()
{
	return Get(_currentImage);
}

Gdiplus::Image* ImageCache::Get(const std::filesystem::path& path)
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

Gdiplus::Image* ImageCache::Load(const std::filesystem::path& path)
{
	Gdiplus::Image* image = Gdiplus::Image::FromFile(path.c_str());

	if (!image)
	{
		return nullptr;
	}

	if (_useCaching)
	{
		CorrectRotationIfNeeded(image);
		_cache[path].reset(image);
		LOGD << L"Cached: " << path;
	}

	return image;
}