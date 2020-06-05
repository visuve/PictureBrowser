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

ImageCache::~ImageCache()
{
	Clear();
}

Gdiplus::Image* ImageCache::Get(const std::filesystem::path& path)
{
	const auto iter = m_cache.find(path);

	if (iter != m_cache.cend())
	{
		return iter->second.get();
	}

	Gdiplus::Image* image = Gdiplus::Image::FromFile(path.c_str());

	if (!image)
	{
		return nullptr;
	}

	CorrectRotationIfNeeded(image);
	m_cache[path].reset(image);
	LOGD << L"Cached: " << path;
	return image;
}

void ImageCache::Clear()
{
	m_cache.clear();
}
