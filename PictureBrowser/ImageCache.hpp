#pragma once

namespace PictureBrowser
{
	class ImageCache
	{
	public:
		ImageCache() = default;
		ImageCache(bool useCaching);
		~ImageCache();

		bool SetCurrent(const std::filesystem::path& path);
		ComPtr<ID2D1Bitmap> Current();
		ComPtr<ID2D1Bitmap> Get(const std::filesystem::path& path);
		bool Delete(const std::filesystem::path& path);

		void Clear();

		// TODO: I hate this function
		inline void SetRenderTarget(ID2D1RenderTarget* renderTarget)
		{
			_renderTarget = renderTarget;
		}

	private:
		ComPtr<ID2D1Bitmap> Load(const std::filesystem::path& path);

		std::map<std::filesystem::path, ComPtr<ID2D1Bitmap>> _cache = { { L"", nullptr } };
		std::filesystem::path _currentImage;
		bool _useCaching = true;

		ID2D1RenderTarget* _renderTarget = nullptr;
		ComPtr<IWICImagingFactory> _wicFactory;
	};
}