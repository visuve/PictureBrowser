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
		bool Listen();

		std::wstring Text() const;

		RECT WindowRect() const;
		SIZE WindowSize() const;
		RECT ClientRect() const;
		SIZE ClientSize() const;
		bool SetPosition(HWND z, int x, int y, int w, int h, UINT flags);
		LRESULT Send(UINT message, WPARAM wParam, LPARAM lParam) const;

		virtual void HandleMessage(UINT, WPARAM, LPARAM);

	protected:
		HWND _parent = nullptr;
		HWND _window = nullptr;
	};
};