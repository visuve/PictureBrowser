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
		PAINTSTRUCT _paintScruct = { 0 };
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

	void ScaleAndCenterTo(const Gdiplus::Size& source, const Gdiplus::Size& size, Gdiplus::Rect& dest);
	void Zoom(Gdiplus::Rect& rect, int zoomPercent);

	Gdiplus::RotateFlipType PropertyToRotateFlipType(Gdiplus::PropertyItem* prop);
	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image);
};

