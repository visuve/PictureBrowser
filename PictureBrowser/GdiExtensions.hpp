#pragma once

namespace GdiExtensions
{
	class ContextWrapper
	{
	public:
		ContextWrapper(HWND window);
		~ContextWrapper();

		bool IsValid() const;
		Gdiplus::Graphics& Get();

	private:
		const HWND m_window;
		const HDC m_deviceContext;
		PAINTSTRUCT m_paintScruct = { 0 };
		Gdiplus::Graphics m_graphics;
	};

	class PropertyWrapper
	{
	public:
		PropertyWrapper(Gdiplus::Image* image, PROPID propertyId);
		~PropertyWrapper();

		bool IsValid() const;
		Gdiplus::PropertyItem* Get() const;

	private:
		Gdiplus::Status m_status = Gdiplus::Status::PropertyNotFound;
		Gdiplus::PropertyItem* m_property = nullptr;
	};

	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, float imageWidth, float imageHeight);
	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, UINT imageWidth, UINT imageHeight);
	
	void Zoom(Gdiplus::Rect& rect, int zoomPercent);

	Gdiplus::RotateFlipType PropertyToRotateFlipType(Gdiplus::PropertyItem* prop);
	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image);
};

