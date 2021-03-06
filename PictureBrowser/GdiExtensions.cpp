#include "PCH.hpp"
#include "GdiExtensions.hpp"

namespace GdiExtensions
{
	Gdiplus::RotateFlipType GetRotation(Gdiplus::Image* image)
	{
		const PropertyWrapper prop(image, PropertyTagOrientation);

		if (!prop.IsValid())
		{
			return Gdiplus::RotateFlipType::RotateNoneFlipNone;
		}

		return PropertyToRotateFlipType(prop.Get());
	}

	ContextWrapper::ContextWrapper(HWND window) :
		m_window(window),
		m_deviceContext(BeginPaint(m_window, &m_paintScruct)),
		m_graphics(m_deviceContext)
	{
	}

	ContextWrapper::~ContextWrapper()
	{
		if (m_deviceContext)
		{
			EndPaint(m_window, &m_paintScruct);
		}
	}

	bool ContextWrapper::IsValid() const
	{
		return m_deviceContext;
	}

	Gdiplus::Graphics& ContextWrapper::Graphics()
	{
		return m_graphics;
	}

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

	void ScaleAndCenterTo(const Gdiplus::Rect& source, const Gdiplus::SizeF& size, Gdiplus::Rect& dest)
	{
		float canvasWidth = static_cast<float>(source.Width);
		float canvasHeight = static_cast<float>(source.Height);

		float aspectRatio = min(
			canvasWidth / size.Width,
			canvasHeight / size.Height);

		dest.X = int(canvasWidth - size.Width * aspectRatio) / 2;
		dest.Y = int(canvasHeight - size.Height * aspectRatio) / 2;
		dest.Width = int(size.Width * aspectRatio);
		dest.Height = int(size.Height * aspectRatio);
	}

	void Zoom(Gdiplus::Rect& rect, int zoomPercent)
	{
		if (zoomPercent > 0)
		{
			rect.X -= int(rect.Width * (zoomPercent / 200.0));
			rect.Y -= int(rect.Height * (zoomPercent / 200.0));
			rect.Width += int(rect.Width * (zoomPercent / 100.0));
			rect.Height += int(rect.Height * (zoomPercent / 100.0));
		}
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
}