#pragma once

#include "BaseWindow.hpp"
#include "Widget.hpp"

namespace PictureBrowser
{
	class Window : public BaseWindow
	{
	public:
		Window(
			HINSTANCE instance,
			const wchar_t* className,
			const wchar_t* title,
			int width,
			int height,
			HICON icon = nullptr,
			HCURSOR cursor = nullptr,
			HBRUSH brush = nullptr,
			const wchar_t* menuName = nullptr);

		virtual ~Window();

		void Show(int showCommand);

	protected:
		static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		HINSTANCE Instance() const;

		Widget AddWidget(
			const wchar_t* className,
			const wchar_t* windowName,
			DWORD style,
			int x,
			int y,
			int w,
			int h,
			HMENU menu = nullptr,
			DWORD extraStyle = 0);

		INT_PTR DialogBoxParamW(const wchar_t* templateName, DLGPROC callback, LPARAM initialParameter = 0) const;

	private:
		CREATESTRUCT _windowStyle;
	};
}