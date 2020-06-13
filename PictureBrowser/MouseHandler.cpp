#include "PCH.hpp"
#include "MouseHandler.hpp"
#include "LogWrap.hpp"

MouseHandler::MouseHandler(HWND window, HWND canvas, const std::function<void()>& invalidate) :
	m_window(window),
	m_canvas(canvas),
	m_invalidate(invalidate)
{
}

void MouseHandler::OnLeftMouseDown(LPARAM lParam)
{
	const POINT point = { LOWORD(lParam), HIWORD(lParam) };
	m_isDragging = DragDetect(m_canvas, point);

	if (!m_isDragging)
	{
		return;
	}

	m_mouseDragStart.X = point.x - m_mouseDragOffset.X;
	m_mouseDragStart.Y = point.y - m_mouseDragOffset.Y;
}

void MouseHandler::OnMouseMove(LPARAM lParam)
{
	if (!m_isDragging)
	{
		return;
	}

	if (UpdateMousePosition(lParam))
	{
		m_invalidate();
	}
}

bool MouseHandler::UpdateMousePosition(LPARAM lParam)
{
	Gdiplus::Point distance(LOWORD(lParam) - m_mouseDragStart.X, HIWORD(lParam) - m_mouseDragStart.Y);

	if (distance.Equals(m_mouseDragOffset))
	{
		return false;
	}

	std::swap(m_mouseDragOffset, distance);
	return true;
}

void MouseHandler::OnLeftMouseUp(LPARAM)
{
	m_isDragging = false;
	m_invalidate();
}

void MouseHandler::OnDoubleClick()
{
	WINDOWPLACEMENT placement = { 0 };

	if (!GetWindowPlacement(m_window, &placement))
	{
		return;
	}

	const UINT show = placement.showCmd == SW_NORMAL ? SW_SHOWMAXIMIZED : SW_NORMAL;

	if (!ShowWindow(m_window, show))
	{
		LOGD << L"Failed to " << (show == SW_SHOWMAXIMIZED ? L"maximize screen" : L"show window in normal size");
	}
}

bool MouseHandler::IsDragging() const
{
	return m_isDragging;
}

Gdiplus::Point MouseHandler::MouseDragOffset() const
{
	return m_mouseDragOffset;
}

void MouseHandler::ResetOffsets()
{
	m_mouseDragStart = { 0, 0 };
	m_mouseDragOffset = { 0, 0 };
}
