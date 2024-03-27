#pragma once

#include "ImageCache.hpp"
#include "Widget.hpp"

namespace PictureBrowser
{
	class CanvasWidget : public Widget
	{
	public:
		CanvasWidget() = default;
		CanvasWidget(
			HINSTANCE instance,
			BaseWindow* parent,
			const std::shared_ptr<ImageCache>& imageCache);

		void HandleMessage(UINT, WPARAM, LPARAM) override;

		void OnImageChanged(const std::filesystem::path& path);

		void Resize();

	private:
		void OnPaint() const;
		void Invalidate() const;
		void OnZoom(WPARAM);

		void OnLeftMouseDown(LPARAM);
		void OnMouseMove(LPARAM);
		bool UpdateMousePosition(LPARAM);
		void OnLeftMouseUp(LPARAM);

		float _zoomPercent = 0.0f;
		bool _isDragging = false;
		D2D_POINT_2F _mouseDragStart = { 0.0f, 0.0f };
		D2D_POINT_2F _mouseDragOffset = { 0.0f, 0.0f };
		std::shared_ptr<ImageCache> _imageCache;

		ComPtr<ID2D1Factory> _factory;
		ComPtr<ID2D1HwndRenderTarget> _renderTarget;
		ComPtr<ID2D1SolidColorBrush> _brush;
	};
}

