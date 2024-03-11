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
		BaseWindow* parent,
		HMENU menu,
		HINSTANCE instance,
		void* data) :
		BaseWindow(CreateWindowExW(
			extraStyle,
			className,
			windowName,
			style,
			x,
			y,
			w,
			h,
			*parent,
			menu,
			instance,
			data)),
		_parent(parent)
	{
		_ASSERTE(_self != nullptr);
	}

	void Widget::Intercept(BaseWindow* window)
	{
		window->SetWindowSubclass(SubClassProcedure, ++Identifier, reinterpret_cast<DWORD_PTR>(this));
	}

	void Widget::Listen()
	{
		Intercept(this);
	}

	void Widget::HandleMessage(UINT, WPARAM, LPARAM)
	{
	}
}