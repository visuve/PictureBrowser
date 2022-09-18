#pragma once

namespace PictureBrowser::GdiExtensions
{
	class Environment
	{
	public:
		Environment();
		~Environment();

		operator bool() const;

	private:
		Gdiplus::GdiplusStartupInput _gdiPlusStartupInput;
		ULONG_PTR _gdiPlusToken = 0;
		Gdiplus::Status _status;
	};

	class ContextWrapper
	{
	public:
		ContextWrapper(HWND window);
		~ContextWrapper();

		bool IsValid() const;
		Gdiplus::Graphics& Graphics();

	private:
		const HWND _window;
		const HDC _deviceContext;
		PAINTSTRUCT _paintScruct = { };
		Gdiplus::Graphics _graphics;
	};

	class PropertyWrapper
	{
	public:
		PropertyWrapper(Gdiplus::Image* image, PROPID propertyId);
		~PropertyWrapper();

		bool IsValid() const;
		Gdiplus::PropertyItem* Get() const;

	private:
		Gdiplus::Status _status = Gdiplus::Status::PropertyNotFound;
		Gdiplus::PropertyItem* _property = nullptr;
	};

	constexpr void ScaleAndCenterTo(const Gdiplus::Size& canvas, const Gdiplus::Size& image, Gdiplus::Rect& dest)
	{
		const float canvasWidth = static_cast<float>(canvas.Width);
		const float canvasHeight = static_cast<float>(canvas.Height);
		const float imageWidth = static_cast<float>(image.Width);
		const float imageHeight = static_cast<float>(image.Height);

		float aspectRatio = min(
			canvasWidth / imageWidth,
			canvasHeight / imageHeight);

		dest.X = int(canvasWidth - imageWidth * aspectRatio) / 2;
		dest.Y = int(canvasHeight - imageHeight * aspectRatio) / 2;
		dest.Width = int(imageWidth * aspectRatio);
		dest.Height = int(imageHeight * aspectRatio);
	}

	constexpr void Zoom(Gdiplus::Rect& rect, int zoomPercent)
	{
		if (zoomPercent > 0)
		{
			rect.X -= int(rect.Width * (zoomPercent / 200.0));
			rect.Y -= int(rect.Height * (zoomPercent / 200.0));
			rect.Width += int(rect.Width * (zoomPercent / 100.0));
			rect.Height += int(rect.Height * (zoomPercent / 100.0));
		}
	}

	Gdiplus::RotateFlipType PropertyToRotateFlipType(Gdiplus::PropertyItem* prop);
	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image);
};

