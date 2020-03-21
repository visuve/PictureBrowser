#include "PCH.hpp"
#include "GdiExtensions.hpp"

namespace GdiExtensions
{
	PropertyWrapper::PropertyWrapper(Gdiplus::Image* image, PROPID propertyId)
	{
		UINT propertySize = image->GetPropertyItemSize(propertyId);

		if (propertySize)
		{
			m_property = reinterpret_cast<Gdiplus::PropertyItem*>(malloc(propertySize));
			m_status = image->GetPropertyItem(propertyId, propertySize, m_property);
		}
	}

	PropertyWrapper::~PropertyWrapper()
	{
		if (m_property)
		{
			free(m_property);
			m_property = nullptr;
		}
	}

	bool PropertyWrapper::IsValid() const
	{
		return m_property && m_status == Gdiplus::Status::Ok;
	}

	Gdiplus::PropertyItem* PropertyWrapper::Get() const
	{
		return m_property;
	}

	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, float imageWidth, float imageHeight)
	{
		float canvasWidth = static_cast<float>(canvasSize.right - canvasSize.left);
		float canvasHeight = static_cast<float>(canvasSize.bottom - canvasSize.top);

		float aspectRatio = min(
			canvasWidth / imageWidth,
			canvasHeight / imageHeight);

		UINT x = UINT(canvasWidth - imageWidth * aspectRatio) / 2;
		UINT y = UINT(canvasHeight - imageHeight * aspectRatio) / 2;
		UINT w = UINT(imageWidth * aspectRatio);
		UINT h = UINT(imageHeight * aspectRatio);

		return Gdiplus::Rect(x + canvasSize.left, y + canvasSize.top, w, h);
	}

	Gdiplus::Rect ScaleToCanvasSize(const RECT& canvasSize, UINT imageWidth, UINT imageHeight)
	{
		return ScaleToCanvasSize(canvasSize, float(imageWidth), float(imageHeight));
	}

	Gdiplus::RotateFlipType PropertyToRotateFlipType(Gdiplus::PropertyItem* prop)
	{
		uint16_t* ptr = reinterpret_cast<uint16_t*>(prop->value);
		uint16_t value = static_cast<uint16_t>(*ptr);

		switch (value)
		{
			case 1:
				return Gdiplus::RotateFlipType::RotateNoneFlipNone;
			case 2:
				return Gdiplus::RotateFlipType::RotateNoneFlipX;
			case 3:
				return Gdiplus::RotateFlipType::Rotate180FlipNone;
			case 4:
				return Gdiplus::RotateFlipType::Rotate180FlipX;
			case 5:
				return Gdiplus::RotateFlipType::Rotate90FlipX;
			case 6:
				return Gdiplus::RotateFlipType::Rotate90FlipNone;
			case 7:
				return Gdiplus::RotateFlipType::Rotate270FlipX;
			case 8:
				return Gdiplus::RotateFlipType::Rotate270FlipNone;
		}

		return Gdiplus::RotateFlipType::RotateNoneFlipNone;
	}

	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image)
	{
		const PropertyWrapper prop(image, PropertyTagOrientation);

		if (!prop.IsValid())
		{
			return Gdiplus::RotateFlipType::RotateNoneFlipNone;
		}

		return PropertyToRotateFlipType(prop.Get());
	}
}