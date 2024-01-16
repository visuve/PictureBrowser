#include "PCH.hpp"
#include "ImageCache.hpp"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	ImageCache::ImageCache(bool useCaching) :
		_useCaching(useCaching)
	{
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&_wicFactory));

		if (FAILED(hr))
		{
			std::unreachable();
		}
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

	ComPtr<ID2D1Bitmap> ImageCache::Current()
	{
		return Get(_currentImage);
	}

	ComPtr<ID2D1Bitmap> ImageCache::Get(const std::filesystem::path& path)
	{
		if (_useCaching)
		{
			const auto iter = _cache.find(path);

			if (iter != _cache.end())
			{
				return iter->second;
			}

			LOGD << L"Not cached: " << path;
		}

		return Load(path);
	}

	bool ImageCache::Delete(const std::filesystem::path& path)
	{
		if (_useCaching)
		{
			const auto iter = _cache.find(path);

			if (iter == _cache.end())
			{
				std::unreachable();
			}

			_cache.erase(iter);
		}

		return DeleteFileW(path.c_str());
	}

	void ImageCache::Clear()
	{
		_cache.clear();
	}

	// TODO: this method should be cleaned up a bit
	ComPtr<ID2D1Bitmap> ImageCache::Load(const std::filesystem::path& path)
	{
		if (!_renderTarget)
		{
			std::unreachable();
		}

		ComPtr<IWICBitmapDecoder> decoder;

		HRESULT hr = _wicFactory->CreateDecoderFromFilename(
			path.c_str(),
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&decoder);

		if (FAILED(hr))
		{
			return nullptr;
		}

		ComPtr<IWICBitmapFrameDecode> frame;

		hr = decoder->GetFrame(0, &frame);

		if (FAILED(hr))
		{
			return nullptr;
		}

		ComPtr<IWICFormatConverter> formatConverter;

		hr = _wicFactory->CreateFormatConverter(&formatConverter);

		if (FAILED(hr))
		{
			return nullptr;
		}

		// I wonder why GUID_WICPixelFormat24bppBGR does not work

		hr = formatConverter->Initialize(
			frame.Get(),
			GUID_WICPixelFormat32bppBGR,
			WICBitmapDitherTypeNone,
			nullptr,
			0.0f,
			WICBitmapPaletteTypeCustom);

		if (FAILED(hr))
		{
			return nullptr;
		}

		ComPtr<ID2D1Bitmap> bitmap;
		
		hr = _renderTarget->CreateBitmapFromWicBitmap(
			formatConverter.Get(),
			nullptr,
			&bitmap);

		if (FAILED(hr))
		{
			return nullptr;
		}

		// TODO: rotate the image if it has rotation flags set

		if (_useCaching)
		{
			_cache.emplace(path, bitmap);
			LOGD << L"Cached: " << path;
		}

		return bitmap;
	}

}