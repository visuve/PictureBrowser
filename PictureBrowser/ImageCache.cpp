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
	m_useCaching(useCaching)
{
}

ImageCache::~ImageCache()
{
	Clear();
}

bool ImageCache::SetCurrent(const std::filesystem::path& path)
{
	if (!Load(path))
	{
		return false;
	}

	m_currentImage = path;
	return true;
}

Gdiplus::Image* ImageCache::Current()
{
	return Get(m_currentImage);
}

Gdiplus::Image* ImageCache::Get(const std::filesystem::path& path)
{
	if (m_useCaching)
	{
		const auto iter = m_cache.find(path);

		if (iter != m_cache.cend())
		{
			return iter->second.get();
		}
	}

	return Load(path);
}

void ImageCache::Clear()
{
	m_cache.clear();
}

Gdiplus::Image* ImageCache::Load(const std::filesystem::path& path)
{
	Gdiplus::Image* image = Gdiplus::Image::FromFile(path.c_str());

	if (!image)
	{
		return nullptr;
	}

	if (m_useCaching)
	{
		CorrectRotationIfNeeded(image);
		m_cache[path].reset(image);
		LOGD << L"Cached: " << path;
	}

	return image;
}