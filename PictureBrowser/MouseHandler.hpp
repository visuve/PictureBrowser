#pragma once

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
	const HWND m_window = nullptr;
	const HWND m_canvas = nullptr;
	std::function<void()> m_invalidate;

	bool m_isDragging = false;
	Gdiplus::Point m_mouseDragStart;
	Gdiplus::Point m_mouseDragOffset;
};