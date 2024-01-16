#include "PCH.hpp"
#include "Widget.hpp"

namespace PictureBrowser
{
	UINT_PTR Identifier = 0;

	LRESULT CALLBACK SubClassProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uid, DWORD_PTR data)
	{
		if (message == WM_NCDESTROY)
		{
			RemoveWindowSubclass(window, SubClassProcedure, uid);
		}
		else
		{
			Widget* self = reinterpret_cast<Widget*>(data);

			if (self)
			{
				self->HandleMessage(message, wParam, lParam);
			}
		}

		return DefSubclassProc(window, message, wParam, lParam);
	}

	Widget::Widget(
		DWORD extraStyle,
		const wchar_t* className,
		const wchar_t* windowName,
		DWORD style,
		int x,
		int y,
		int w,
		int h,
		HWND parent,
		HMENU menu,
		HINSTANCE instance,
		void* data) :
		_window(CreateWindowExW(
			extraStyle,
			className,
			windowName,
			style,
			x,
			y,
			w,
			h,
			parent,
			menu,
			instance,
			data)),
		_parent(parent)
	{
		_ASSERTE(_window != nullptr);
	}

	bool Widget::Intercept(HWND window)
	{
		return SetWindowSubclass(window, SubClassProcedure, ++Identifier, reinterpret_cast<DWORD_PTR>(this));
	}

	bool Widget::Intercept(Widget* widget)
	{
		return Intercept(widget->_window);
	}

	bool Widget::Listen()
	{
		return Intercept(this);
	}

	std::wstring Widget::Text() const
	{
		std::wstring buffer;
		int length = GetWindowTextLength(_window);

		if (length > 0)
		{
			buffer.resize(length);
			GetWindowText(_window, &buffer.front(), length + 1);
		}

		return buffer;
	}

	RECT Widget::WindowRect() const
	{ 
		RECT area;

		if (!GetWindowRect(_window, &area))
		{
			std::unreachable();
		}

		return area;
	}

	SIZE Widget::WindowSize() const
	{
		RECT area = WindowRect();
		return { area.right - area.left, area.bottom - area.top };
	}

	RECT Widget::ClientRect() const
	{
		RECT area;

		if (!GetClientRect(_window, &area))
		{
			std::unreachable();
		}

		return area;
	}

	SIZE Widget::ClientSize() const
	{
		RECT area = ClientRect();
		return { area.right - area.left, area.bottom - area.top };
	}

	bool Widget::SetPosition(HWND z, int x, int y, int w, int h, UINT flags)
	{
		return SetWindowPos(_window, z, x, y, w, h, flags);
	}

	LRESULT Widget::Send(UINT message, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(_window, message, wParam, lParam);
	}

	bool Widget::Post(UINT message, WPARAM wParam, LPARAM lParam) const
	{
		return PostMessageW(_window, message, wParam, lParam);
	}

	void Widget::HandleMessage(UINT, WPARAM, LPARAM)
	{
	}
}