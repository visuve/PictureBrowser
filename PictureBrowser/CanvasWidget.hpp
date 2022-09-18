#pragma once

#include "ImageCache.hpp"
#include "Widget.hpp"

namespace PictureBrowser
{
	class CanvasWidget : public Widget
	{
	public:
		CanvasWidget(
			HINSTANCE instance, 
			HWND parent,
			const std::shared_ptr<ImageCache>& imageCache);

		void HandleMessage(UINT, WPARAM, LPARAM) override;

		void OnImageChanged(std::filesystem::path path);

		void Resize();

	private:
		void OnPaint() const;
		void Invalidate() const;
		void OnZoom(WPARAM);

		void OnLeftMouseDown(LPARAM);
		void OnMouseMove(LPARAM);
		bool UpdateMousePosition(LPARAM);
		void OnLeftMouseUp(LPARAM);

		int _zoomPercent = 0;
		bool _isDragging = false;
		Gdiplus::Point _mouseDragStart;
		Gdiplus::Point _mouseDragOffset;
		std::shared_ptr<ImageCache> _imageCache;
		std::unique_ptr<Gdiplus::Bitmap> _background;
		std::unique_ptr<Gdiplus::Graphics> _graphics;
	};
}

