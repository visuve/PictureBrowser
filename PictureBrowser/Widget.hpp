#pragma once

namespace PictureBrowser
{
	class Widget
	{
	public:
		Widget() = default;

		Widget(
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
			void* data);

		virtual ~Widget() = default;
		
		bool Intercept(HWND window);
		bool Intercept(Widget* other);

		std::wstring Text() const;
		LRESULT Send(UINT message, WPARAM wParam, LPARAM lParam) const;

		virtual void HandleMessage(UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

	private:
		HWND _window = nullptr;
	};
};