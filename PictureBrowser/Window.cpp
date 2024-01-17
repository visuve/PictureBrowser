#include "PCH.hpp"
#include "Window.hpp"

namespace PictureBrowser
{
	Window::Window(
		HINSTANCE instance, 
		const wchar_t* className, 
		const wchar_t* title, 
		int width, 
		int height, 
		HICON icon, 
		HCURSOR cursor, 
		HBRUSH brush, 
		const wchar_t* menuName)
	{
		// TODO: add more ctor parameters to allow more flexible usage

		WNDCLASSEXW windowClass = { };
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		windowClass.lpfnWndProc = WindowProcedure;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = instance;
		windowClass.hIcon = icon;
		windowClass.hCursor = cursor;
		windowClass.hbrBackground = brush;
		windowClass.lpszMenuName = menuName;
		windowClass.lpszClassName = className;
		windowClass.hIconSm = nullptr;

		ATOM atom = RegisterClassExW(&windowClass);

		_ASSERTE(atom != 0);

		_windowStyle.lpCreateParams = this;
		_windowStyle.hInstance = instance;
		_windowStyle.hMenu = nullptr;
		_windowStyle.hwndParent = nullptr;
		_windowStyle.cy = height;
		_windowStyle.cx = width;
		_windowStyle.y = CW_USEDEFAULT;
		_windowStyle.x = CW_USEDEFAULT;
		_windowStyle.style = WS_OVERLAPPEDWINDOW;
		_windowStyle.lpszName = title;
		_windowStyle.lpszClass = MAKEINTATOM(atom);
		_windowStyle.dwExStyle = WS_EX_ACCEPTFILES;
	}

	Window::~Window()
	{
	}

	void Window::Show(int showCommand)
	{
		if (!_window)
		{
			HWND window = CreateWindowExW(
				_windowStyle.dwExStyle,
				_windowStyle.lpszClass,
				_windowStyle.lpszName,
				_windowStyle.style,
				_windowStyle.x,
				_windowStyle.y,
				_windowStyle.cx,
				_windowStyle.cy,
				_windowStyle.hwndParent,
				_windowStyle.hMenu,
				_windowStyle.hInstance,
				_windowStyle.lpCreateParams);

			// WindowProcedure triggers immediately after CreateWindowEx.
			// If you call CreateWindowEx before the Window class is fully initialized
			// a crash will follow in WindowProcedure.

			if (window == nullptr || window != _window)
			{
				throw std::runtime_error("CreateWindowExW failed!");
			}
		}

		if (_window)
		{
			ShowWindow(_window, showCommand);
		}
	}

	LRESULT Window::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* self = nullptr;

		if (message == WM_CREATE)
		{
			auto created = reinterpret_cast<CREATESTRUCT*>(lParam);
			self = static_cast<Window*>(created->lpCreateParams);
			self->_window = window;
			SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
		}
		else
		{
			LONG_PTR userData = GetWindowLongPtr(window, GWLP_USERDATA);
			self = reinterpret_cast<Window*>(userData);
		}

		if (self && self->HandleMessage(message, wParam, lParam))
		{
			return S_OK;
		}

		return DefWindowProc(window, message, wParam, lParam);
	}

	HINSTANCE Window::Instance() const
	{
		return _windowStyle.hInstance;
	}

	Widget Window::AddWidget(const wchar_t* className, const wchar_t* windowName, DWORD style, int x, int y, int w, int h, HMENU menu, DWORD extraStyle)
	{
		return Widget(extraStyle, className, windowName, style, x, y, w, h, _window, menu, Instance(), nullptr);
	}

}