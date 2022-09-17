#include "PCH.hpp"
#include "MouseHandler.hpp"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	MouseHandler::MouseHandler(HWND window, HWND canvas, const std::function<void()>& invalidate) :
		_window(window),
		_canvas(canvas),
		_invalidate(invalidate)
	{
	}

	void MouseHandler::OnLeftMouseDown(LPARAM lParam)
	{
		const POINT point = { LOWORD(lParam), HIWORD(lParam) };
		_isDragging = DragDetect(_canvas, point);

		if (!_isDragging)
		{
			return;
		}

		_mouseDragStart.X = point.x - _mouseDragOffset.X;
		_mouseDragStart.Y = point.y - _mouseDragOffset.Y;
	}

	void MouseHandler::OnMouseMove(LPARAM lParam)
	{
		if (!_isDragging)
		{
			return;
		}

		if (UpdateMousePosition(lParam))
		{
			_invalidate();
		}
	}

	bool MouseHandler::UpdateMousePosition(LPARAM lParam)
	{
		Gdiplus::Point distance(LOWORD(lParam) - _mouseDragStart.X, HIWORD(lParam) - _mouseDragStart.Y);

		if (distance.Equals(_mouseDragOffset))
		{
			return false;
		}

		std::swap(_mouseDragOffset, distance);
		return true;
	}

	void MouseHandler::OnLeftMouseUp(LPARAM)
	{
		_isDragging = false;
		_invalidate();
	}

	void MouseHandler::OnDoubleClick()
	{
		WINDOWPLACEMENT placement = {};

		if (!GetWindowPlacement(_window, &placement))
		{
			return;
		}

		const UINT show = placement.showCmd == SW_NORMAL ? SW_SHOWMAXIMIZED : SW_NORMAL;

		if (!ShowWindow(_window, show))
		{
			const std::wstring message =
				show == SW_SHOWMAXIMIZED ? L"maximize screen" : L"show window in normal size";
			LOGD << L"Failed to " << message;
		}
	}

	bool MouseHandler::IsDragging() const
	{
		return _isDragging;
	}

	Gdiplus::Point MouseHandler::MouseDragOffset() const
	{
		return _mouseDragOffset;
	}

	void MouseHandler::ResetOffsets()
	{
		Clear(&_mouseDragStart);
		Clear(&_mouseDragOffset);
	}
}