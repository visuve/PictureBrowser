#pragma once

namespace PictureBrowser
{
	class MouseHandler
	{
	public:
		MouseHandler(HWND window, HWND canvas, const std::function<void()>& invalidate);

		void OnLeftMouseDown(LPARAM);
		void OnMouseMove(LPARAM);
		bool UpdateMousePosition(LPARAM);
		void OnLeftMouseUp(LPARAM);
		void OnDoubleClick();

		bool IsDragging() const;
		Gdiplus::Point MouseDragOffset() const;
		void ResetOffsets();

	private:
		const HWND _window = nullptr;
		const HWND _canvas = nullptr;
		std::function<void()> _invalidate;

		bool _isDragging = false;
		Gdiplus::Point _mouseDragStart;
		Gdiplus::Point _mouseDragOffset;
	};
}