#include "PCH.hpp"
#include "CanvasWidget.hpp"
#include "Resource.h"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	constexpr D2D_RECT_F ScaleAndCenterTo(const D2D_SIZE_F& canvas, const D2D_SIZE_F& image)
	{
		float aspectRatio = std::min(
			canvas.width / image.width,
			canvas.height / image.height);

		float w = image.width * aspectRatio;
		float h = image.height * aspectRatio;

		float x = (canvas.width - w) / 2.0f;
		float y = (canvas.height - h) / 2.0f;

		return { x, y, w + x, h + y };
	}

	constexpr void Zoom(D2D_RECT_F& rect, float zoomPercent)
	{
		if (zoomPercent > 0)
		{
			float width = rect.right - rect.left;
			float height = rect.bottom - rect.top;
			float scale = zoomPercent / 100.0f;

			rect.left -= width * scale;
			rect.top -= height * scale;
			rect.right += width * scale;
			rect.bottom += height * scale;
		}
	}

	thread_local PAINTSTRUCT Paint;

	class PaintGuard
	{
	public:
		PaintGuard(const BaseWindow* widget, ID2D1HwndRenderTarget* target) :
			_widget(widget),
			_target(target)
#ifdef _DEBUG
			, _start(std::chrono::high_resolution_clock::now())
#endif
		{
			_ASSERTE(_widget);
			_ASSERTE(_target);

			_widget->BeginPaint(Paint);
			_target->BeginDraw();
		}

		~PaintGuard()
		{
			_target->EndDraw();
			_widget->EndPaint(Paint);
#ifdef _DEBUG
			const auto diff = std::chrono::high_resolution_clock::now() - _start;
			LOGD << std::chrono::duration_cast<std::chrono::microseconds>(diff).count() << L"us";
#endif
		}

	private:
		const BaseWindow* _widget;
		ID2D1HwndRenderTarget* _target;

#ifdef _DEBUG
		const std::chrono::high_resolution_clock::time_point _start;
#endif
	};

	CanvasWidget::CanvasWidget(HINSTANCE instance, BaseWindow* parent, const std::shared_ptr<ImageCache>& imageCache) :
		Widget(
			0,
			WC_STATIC,
			ClassName(CanvasWidget),
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
		HRESULT hr;

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, _factory.GetAddressOf());

		if (FAILED(hr))
		{
			throw std::system_error(hr, std::system_category(), "D2D1CreateFactory");
		}

		RECT rect = GetClientRect();

		D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);

		auto rtp = D2D1::RenderTargetProperties();
		auto hrtp = D2D1::HwndRenderTargetProperties(*this, size);

		hr = _factory->CreateHwndRenderTarget(rtp, hrtp, &_renderTarget);
		
		if (FAILED(hr))
		{
			throw std::system_error(hr, std::system_category(),"ID2D1Factory::CreateHwndRenderTarget");
		}

		// TODO: I really do not like this, but I could not come up with else
		_imageCache->SetRenderTarget(_renderTarget.Get());

		_renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &_brush);
	}

	bool CanvasWidget::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
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

		return Widget::HandleMessage(message, wParam, lParam);
	}

	void CanvasWidget::Resize()
	{
		if (!_renderTarget)
		{
			return;
		}

		SIZE size = GetClientSize();

		_renderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
	}

	void CanvasWidget::OnPaint() const
	{
		PaintGuard paintGuard(this, _renderTarget.Get());

		ComPtr<ID2D1Bitmap> bitmap = _imageCache->Current();

		_renderTarget->Clear(bitmap ? D2D1::ColorF(D2D1::ColorF::Gray) : D2D1::ColorF(D2D1::ColorF::Red));

		if (bitmap)
		{
			D2D_SIZE_F canvasSize = _renderTarget->GetSize();
			D2D_SIZE_F imageSize = bitmap->GetSize();
			D2D_RECT_F scaled = ScaleAndCenterTo(canvasSize, imageSize);

			scaled.left += _mouseDragOffset.x;
			scaled.top += _mouseDragOffset.y;
			scaled.right += _mouseDragOffset.x;
			scaled.bottom += _mouseDragOffset.y;

			Zoom(scaled, _zoomPercent);

			if (_isDragging)
			{
				_renderTarget->FillRectangle(scaled, _brush.Get());
			}
			else
			{
				_renderTarget->DrawBitmap(bitmap.Get(), scaled);
			}
		}
	}

	void CanvasWidget::Invalidate() const
	{
		RECT child = GetClientRect();

		constexpr size_t points = sizeof(RECT) / sizeof(POINT);

		MapWindowPoints(_parent, { reinterpret_cast<POINT*>(&child), points });

		_parent->InvalidateRect(child, false);
	}

	void CanvasWidget::OnImageChanged(const std::filesystem::path& path)
	{
		_zoomPercent = 0.0f;

		ZeroInit(_mouseDragStart);
		ZeroInit(_mouseDragOffset);

		Invalidate();

		// TODO: I really do not like that the child sets the title
		const std::wstring title = L"Picture Browser 2.2 - " + path.filename().wstring();
		_parent->SetWindowTextW(title.c_str());
	}

	void CanvasWidget::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (_zoomPercent > 0.0f)
				{
					_zoomPercent -= 5.0f;
					Invalidate();
				}
				break;
			case VK_OEM_PLUS:
				if (_zoomPercent < 1000.0f)
				{
					_zoomPercent += 5.0f;
					Invalidate();
				}
				break;
		}

		LOGD << _zoomPercent;
	}

	void CanvasWidget::OnLeftMouseDown(LPARAM lParam)
	{
		const POINT point = { LOWORD(lParam), HIWORD(lParam) };
		_isDragging = DragDetect(point);

		if (!_isDragging)
		{
			return;
		}

		_mouseDragStart.x = point.x - _mouseDragOffset.x;
		_mouseDragStart.y = point.y - _mouseDragOffset.y;
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
		D2D_POINT_2F distance(LOWORD(lParam) - _mouseDragStart.x, HIWORD(lParam) - _mouseDragStart.y);

		if (distance.x == _mouseDragOffset.x &&
			distance.y == _mouseDragOffset.y)
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