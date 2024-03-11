#pragma once

namespace PictureBrowser
{
	class BaseWindow
	{
	public:
		BaseWindow() = default;
		BaseWindow(HWND self);
		virtual ~BaseWindow();

		HDC BeginPaint(PAINTSTRUCT& ps) const;
		SIZE GetClientSize() const;
		void DestroyWindow() const;
		bool DragDetect(const POINT& point) const;
		void EndPaint(PAINTSTRUCT& ps) const;
		RECT GetClientRect() const;
		HMENU GetMenu() const;
		WINDOWPLACEMENT GetWindowPlacement() const;
		RECT GetWindowRect() const;
		std::wstring GetWindowTextW() const;
		size_t GetWindowTextLengthW() const;
		void InvalidateRect(const RECT& rect, bool erase) const;
		int MapWindowPoints(BaseWindow* to, std::span<POINT> points) const;
		int MessageBoxW(const wchar_t* text, const wchar_t* caption, UINT type) const;
		void PostMessageW(UINT message, WPARAM wParam, LPARAM lParam) const;
		void ScreenToClient(POINT& point) const;
		void SetFocus() const;
		LRESULT SendMessageW(UINT message, WPARAM wParam, LPARAM lParam) const;
		void SetWindowPos(HWND z, int x, int y, int w, int h, UINT flags) const;
		void SetWindowSubclass(SUBCLASSPROC subClassProcedure, UINT_PTR identifier, DWORD_PTR data) const;
		void SetWindowTextW(const wchar_t* text) const;
		void ShowWindow(int cmd) const;
		SIZE WindowSize() const;

		inline operator HWND() const
		{
			return _self;
		}

		template<typename T>
		bool IsMe(T x) const
		{
			return reinterpret_cast<HWND>(x) == _self;
		}

	protected:
		volatile HWND _self = nullptr;
	};
}