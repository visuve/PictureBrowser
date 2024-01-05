#include "PCH.hpp"
#include "CanvasWidget.hpp"
#include "GdiExtensions.hpp"
#include "Resource.h"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	CanvasWidget::CanvasWidget(HINSTANCE instance, HWND parent, const std::shared_ptr<ImageCache>& imageCache) :
		Widget(
			0,
			WC_STATIC,
			nullptr,
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			700,
			700,
			parent,
			nullptr,
			instance,
			nullptr),
		_imageCache(imageCache)
	{
	}

	void CanvasWidget::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_PAINT:
				OnPaint();
				break;
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case IDC_ZOOM_OUT_BUTTON:
					{
						// TODO: disable button if no image is loaded
						OnZoom(VK_OEM_MINUS);
						break;
					}
					case IDC_ZOOM_IN_BUTTON:
					{
						// TODO: disable button if no image is loaded
						OnZoom(VK_OEM_PLUS);
						break;
					}
				}

				break;
			}
			case WM_LBUTTONDOWN:
			{
				OnLeftMouseDown(lParam);
				break;
			}
			case WM_MOUSEMOVE:
			{
				OnMouseMove(lParam);
				break;
			}
			case WM_LBUTTONUP:
			{
				OnLeftMouseUp(lParam);
				break;
			}
		}
	}

	void CanvasWidget::Resize()
	{
		SIZE size = ClientSize();

		_buffer = std::make_unique<Gdiplus::Bitmap>(size.cx, size.cy);
		_graphics = std::make_unique<Gdiplus::Graphics>(_buffer.get());
	}

	void CanvasWidget::OnPaint() const
	{
		const auto start = std::chrono::high_resolution_clock::now();

		GdiExtensions::ContextWrapper context(_window);

		if (!context.IsValid())
		{
			std::unreachable();
		}

		if (!_buffer || !_graphics)
		{
			std::unreachable();
		}

		const INT width = _buffer->GetWidth();
		const INT height = _buffer->GetHeight();

		const Gdiplus::SolidBrush grayBrush(static_cast<Gdiplus::ARGB>(Gdiplus::Color::DarkGray));
		_graphics->FillRectangle(&grayBrush, 0, 0, width, height);

		Gdiplus::Image* image = _imageCache->Current();

		if (image)
		{
			Gdiplus::Size canvasSize(width, height);
			Gdiplus::Size imageSize(image->GetWidth(), image->GetHeight());
			Gdiplus::Rect scaled;

			GdiExtensions::ScaleAndCenterTo(canvasSize, imageSize, scaled);
			GdiExtensions::Zoom(scaled, _zoomPercent);
			scaled.Offset(_mouseDragOffset);

			if (_isDragging)
			{
				const Gdiplus::Pen pen(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Gray), 2.0f);
				_graphics->DrawRectangle(&pen, scaled);
			}
			else
			{
				_graphics->DrawImage(image, scaled);
			}
		}

		context.Graphics().DrawImage(_buffer.get(), 0, 0, width, height);

		const auto diff = std::chrono::high_resolution_clock::now() - start;
		LOGD << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << L"ms";
	}

	void CanvasWidget::Invalidate() const
	{
		RECT child = ClientRect();

		UINT points = sizeof(RECT) / sizeof(POINT);

		MapWindowPoints(_window, _parent, reinterpret_cast<LPPOINT>(&child), points);

		if (!InvalidateRect(_parent, &child, false))
		{
			std::unreachable();
		}
	}

	void CanvasWidget::OnImageChanged(std::filesystem::path path)
	{
		_zoomPercent = 0;

		ZeroInit(&_mouseDragStart);
		ZeroInit(&_mouseDragOffset);

		Invalidate();

		// TODO: I really do not like that the child sets the title
		const std::wstring title = L"Picture Browser 2.2 - " + path.filename().wstring();
		SetWindowText(_parent, title.c_str());
	}

	void CanvasWidget::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (_zoomPercent > 0)
				{
					_zoomPercent -= 5;
					Invalidate();
				}
				break;
			case VK_OEM_PLUS:
				if (_zoomPercent < 1000)
				{
					_zoomPercent += 5;
					Invalidate();
				}
				break;
		}

		LOGD << _zoomPercent;
	}

	void CanvasWidget::OnLeftMouseDown(LPARAM lParam)
	{
		const POINT point = { LOWORD(lParam), HIWORD(lParam) };
		_isDragging = DragDetect(_window, point);

		if (!_isDragging)
		{
			return;
		}

		_mouseDragStart.X = point.x - _mouseDragOffset.X;
		_mouseDragStart.Y = point.y - _mouseDragOffset.Y;
	}

	void CanvasWidget::OnMouseMove(LPARAM lParam)
	{
		if (!_isDragging)
		{
			return;
		}

		if (UpdateMousePosition(lParam))
		{
			Invalidate();
		}
	}

	bool CanvasWidget::UpdateMousePosition(LPARAM lParam)
	{
		Gdiplus::Point distance(LOWORD(lParam) - _mouseDragStart.X, HIWORD(lParam) - _mouseDragStart.Y);

		if (distance.Equals(_mouseDragOffset))
		{
			return false;
		}

		std::swap(_mouseDragOffset, distance);
		return true;
	}

	void CanvasWidget::OnLeftMouseUp(LPARAM)
	{
		_isDragging = false;
		Invalidate();
	}
}