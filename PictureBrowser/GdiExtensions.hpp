#pragma once

namespace GdiExtensions
{
	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, float imageWidth, float imageHeight);
	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, UINT imageWidth, UINT imageHeight);

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

	Gdiplus::RotateFlipType PropertyToRotateFlipType(Gdiplus::PropertyItem* prop);
	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image);
};

