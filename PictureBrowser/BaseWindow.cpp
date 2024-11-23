#include "PCH.hpp"
#include "BaseWindow.hpp"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	BaseWindow::BaseWindow(HWND self) :
		_self(self)
	{
	}

	BaseWindow::~BaseWindow()
	{
		LOGD << L"Bye bye!";
	}

	HDC BaseWindow::BeginPaint(PAINTSTRUCT& ps) const
	{
		_ASSERTE(_self);

		HDC result = ::BeginPaint(_self, &ps);

		if (!result)
		{
			throw std::system_error(GetLastError(), std::system_category(), "BeginPaint");
		}

		return result;
	}

	SIZE BaseWindow::GetClientSize() const
	{
		_ASSERTE(_self);

		RECT area = GetClientRect();
		return { area.right - area.left, area.bottom - area.top };
	}

	void BaseWindow::DestroyWindow() const
	{
		_ASSERTE(_self);

		if (!::DestroyWindow(_self))
		{
			throw std::system_error(GetLastError(), std::system_category(), "DestroyWindow");
		}
	}

	bool BaseWindow::DragDetect(const POINT& point) const
	{
		_ASSERTE(_self);

		return ::DragDetect(_self, point);
	}

	void BaseWindow::EndPaint(const PAINTSTRUCT& ps) const
	{
		_ASSERTE(_self);

		if (!::EndPaint(_self, &ps))
		{
			throw std::system_error(GetLastError(), std::system_category(), "EndPaint");
		}
	}

	RECT BaseWindow::GetClientRect() const
	{
		_ASSERTE(_self);

		RECT clientArea;
		ZeroInit(clientArea);

		if (!::GetClientRect(_self, &clientArea))
		{
			throw std::system_error(GetLastError(), std::system_category(), "GetClientRect");
		}

		return clientArea;
	}

	HMENU BaseWindow::GetMenu() const
	{
		_ASSERTE(_self);

		return ::GetMenu(_self);
	}

	WINDOWPLACEMENT BaseWindow::GetWindowPlacement() const
	{
		_ASSERTE(_self);

		WINDOWPLACEMENT placement;
		ZeroInit(placement);

		if (!::GetWindowPlacement(_self, &placement))
		{
			throw std::system_error(GetLastError(), std::system_category(), "GetWindowPlacement");
		}

		return placement;
	}

	RECT BaseWindow::GetWindowRect() const
	{
		_ASSERTE(_self);

		RECT area;

		if (!::GetWindowRect(_self, &area))
		{
			throw std::system_error(GetLastError(), std::system_category(), "GetWindowRect");
		}

		return area;
	}

	std::wstring BaseWindow::GetWindowTextW() const
	{
		_ASSERTE(_self);

		size_t size = GetWindowTextLengthW();

		std::wstring buffer;
		buffer.resize(size);

		int result = ::GetWindowTextW(_self, buffer.data(), static_cast<int>(size + 1));

		if (result < 0)
		{
			throw std::runtime_error("GetWindowTextW");
		}

		return buffer;
	}

	size_t BaseWindow::GetWindowTextLengthW() const
	{
		_ASSERTE(_self);

		int result = ::GetWindowTextLengthW(_self);

		if (result < 0)
		{
			throw std::runtime_error("GetWindowTextLengthW");
		}

		return static_cast<size_t>(result);
	}

	void BaseWindow::InvalidateRect(const RECT& rect, bool erase) const
	{
		_ASSERTE(_self);

		if (!::InvalidateRect(_self, &rect, erase))
		{
			throw std::system_error(GetLastError(), std::system_category(), "InvalidateRect");
		}
	}

	int BaseWindow::MapWindowPoints(BaseWindow* to, std::span<POINT> points) const
	{
		return ::MapWindowPoints(_self, to->_self, points.data(), static_cast<UINT>(points.size()));
	}

	int BaseWindow::MessageBoxW(const wchar_t* text, const wchar_t* caption, UINT type) const
	{
		_ASSERTE(_self);

		int result = ::MessageBoxW(_self, text, caption, type);

		if (result == 0)
		{
			throw std::system_error(GetLastError(), std::system_category(), "MessageBoxW");
		}

		return result;
	}

	void BaseWindow::PostMessageW(UINT message, WPARAM wParam, LPARAM lParam) const
	{
		_ASSERTE(_self);

		if (!::PostMessageW(_self, message, wParam, lParam))
		{
			throw std::system_error(GetLastError(), std::system_category(), "PostMessageW");
		}
	}

	void BaseWindow::ScreenToClient(POINT& point) const
	{
		_ASSERTE(_self);

		if (!::ScreenToClient(_self, &point))
		{
			throw std::system_error(GetLastError(), std::system_category(), "ScreenToClient");
		}
	}

	void BaseWindow::SetFocus() const
	{
		_ASSERTE(_self);

		::SetFocus(_self);
	}

	LRESULT BaseWindow::SendMessageW(UINT message, WPARAM wParam, LPARAM lParam) const
	{
		_ASSERTE(_self);

		return ::SendMessageW(_self, message, wParam, lParam);
	}

	void BaseWindow::SetWindowPos(HWND z, int x, int y, int w, int h, UINT flags) const
	{
		_ASSERTE(_self);

		if (!::SetWindowPos(_self, z, x, y, w, h, flags))
		{
			throw std::system_error(GetLastError(), std::system_category(), "SetWindowPos");
		}
	}

	void BaseWindow::SetWindowSubclass(SUBCLASSPROC subClassProcedure, UINT_PTR identifier, DWORD_PTR data) const
	{
		_ASSERTE(_self);

		if (!::SetWindowSubclass(_self, subClassProcedure, identifier, data))
		{
			throw std::system_error(GetLastError(), std::system_category(), "SetWindowSubclass");
		}
	}

	void BaseWindow::SetWindowTextW(const wchar_t* text) const
	{
		_ASSERTE(_self);

		if (!::SetWindowTextW(_self, text))
		{
			throw std::system_error(GetLastError(), std::system_category(), "SetWindowTextW");
		}
	}

	void BaseWindow::ShowWindow(int cmd) const
	{
		_ASSERTE(_self);

		::ShowWindow(_self, cmd);
	}

	SIZE BaseWindow::WindowSize() const
	{
		_ASSERTE(_self);

		RECT area = GetWindowRect();
		return { area.right - area.left, area.bottom - area.top };
	}
}