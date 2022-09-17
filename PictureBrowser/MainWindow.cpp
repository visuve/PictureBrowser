#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"
#include "Registry.hpp"

namespace PictureBrowser
{
	constexpr UINT Padding = 5;
	constexpr UINT ButtonWidth = 50;
	constexpr UINT ButtonHeight = 25;
	constexpr UINT FileListWidth = 250;

	MainWindow::MainWindow(HINSTANCE instance) :
		Window(instance, 
			L"PictureBrowser", 
			L"Picture Browser 2.2", 
			800, 
			800,
			LoadIcon(instance, MAKEINTRESOURCE(IDI_PICTURE_BROWSER)),
			LoadCursor(instance, IDC_CROSS),
			reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
			MAKEINTRESOURCEW(IDC_MENU))
	{
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::Open(const std::filesystem::path& path)
	{
		if (_fileListWidget)
		{
			_fileListWidget->Open(path);
		}
	}

	bool MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_CREATE:
			{
				OnCreate();
				break;
			}
			case WM_DESTROY:
			{
				_fileListWidget->Clear();
				PostQuitMessage(0);
				break;
			}
			case WM_SIZE:
			{
				OnResize();
				break;
			}
			case WM_PAINT:
			{
				DefWindowProc(_window, message, wParam, lParam);
				OnPaint();
				break;
			}
			case WM_ERASEBKGND:
			{
				OnErase();
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				_mouseHandler->OnDoubleClick();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				_mouseHandler->OnLeftMouseDown(lParam);
				break;
			}
			case WM_MOUSEMOVE:
			{
				_mouseHandler->OnMouseMove(lParam);
				break;
			}
			case WM_LBUTTONUP:
			{
				_mouseHandler->OnLeftMouseUp(lParam);
				break;
			}
			case WM_COMMAND:
			{
				OnCommand(wParam);
				break;
			}
		}

		return false;
	}

	void MainWindow::RecalculatePaintArea()
	{
		RECT clientArea = { };

		if (!GetClientRect(_window, &clientArea))
		{
			LOGD << L"GetClientRect failed!";
			clientArea.top = 0;
			clientArea.left = 0;
			clientArea.right = 800;
			clientArea.bottom = 600;
		}

		_fileListArea.X = clientArea.left + Padding;
		_fileListArea.Y = clientArea.top + Padding;
		_fileListArea.Width = FileListWidth;
		_fileListArea.Height = clientArea.bottom - Padding;

		_mainArea.X = _fileListArea.GetRight() + Padding;
		_mainArea.Y = clientArea.top + Padding;
		_mainArea.Width = clientArea.right - _fileListArea.GetRight() - Padding * 2;
		_mainArea.Height = clientArea.bottom - Padding * 2;

		_canvasArea.X = _mainArea.X;
		_canvasArea.Y = _mainArea.Y + Padding + ButtonHeight;
		_canvasArea.Width = _mainArea.Width;
		_canvasArea.Height = _mainArea.Height - Padding * 2 - ButtonHeight * 2;

		LOGD << L"File list area: " << _fileListArea;
		LOGD << L"Main area: " << _mainArea;
		LOGD << L"Canvas area: " << _canvasArea;
	}

	void MainWindow::OnCreate()
	{
		RecalculatePaintArea();

		_canvas = CreateWindow(
			WC_STATIC,
			nullptr,
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_canvasArea.X,
			_canvasArea.Y,
			_canvasArea.Width,
			_canvasArea.Height,
			_window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			Instance(),
			nullptr);

		_zoomOutButton = CreateWindow(
			WC_BUTTON,
			L"-",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.X,
			_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			_window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			Instance(),
			nullptr);

		_zoomInButton = CreateWindow(
			WC_BUTTON,
			L"+",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.Width - ButtonWidth,
			_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			_window,
			reinterpret_cast<HMENU>(IDC_ZOOM_IN_BUTTON),
			Instance(),
			nullptr);

		_previousPictureButton = CreateWindow(
			WC_BUTTON,
			L"<",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.X,
			_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			_window,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON),
			Instance(),
			nullptr);

		_nextPictureButton = CreateWindow(
			WC_BUTTON,
			L">",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.Width - ButtonWidth,
			_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			_window,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON),
			Instance(),
			nullptr);

		const bool useCaching = Registry::Get(L"Software\\PictureBrowser\\UseCaching", true);
		SetCheckedState(IDM_OPTIONS_USE_CACHING, useCaching ? MFS_CHECKED : MFS_UNCHECKED);

		_imageCache = std::make_shared<ImageCache>(useCaching);

		_fileListWidget = std::make_unique<FileListWidget>(
			_window,
			_imageCache,
			std::bind(&MainWindow::OnImageChanged, this, std::placeholders::_1));

		_fileListWidget->Intercept(_window);
		
		_mouseHandler = std::make_unique<MouseHandler>(
			_window,
			_canvas,
			std::bind(&MainWindow::Invalidate, this, true));
	}

	void MainWindow::OnResize()
	{
		RecalculatePaintArea();

		if (!_fileListWidget->SetPosition(
			HWND_TOP,
			0,
			0,
			_fileListArea.Width,
			_fileListArea.Height,
			SWP_NOMOVE | SWP_NOZORDER))
		{
			LOGD << L"Failed to move file list!";
		}

		if (!SetWindowPos(
			_canvas,
			HWND_TOP,
			_canvasArea.GetLeft(),
			_canvasArea.GetTop(),
			_canvasArea.Width,
			_canvasArea.Height,
			SWP_NOZORDER))
		{
			LOGD << L"Failed move minus button!";
		}

		if (!SetWindowPos(
			_zoomOutButton,
			HWND_TOP,
			_mainArea.GetLeft(),
			_mainArea.GetTop(),
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move minus button!";
		}

		if (!SetWindowPos(
			_zoomInButton,
			HWND_TOP,
			_mainArea.GetRight() - ButtonWidth,
			_mainArea.GetTop(),
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move plus button!";
		}

		if (!SetWindowPos(
			_previousPictureButton,
			HWND_TOP,
			_mainArea.GetLeft(),
			_mainArea.GetBottom() - ButtonHeight,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move previous button!";
		}

		if (!SetWindowPos(
			_nextPictureButton,
			HWND_TOP,
			_mainArea.GetRight() - ButtonWidth,
			_mainArea.GetBottom() - ButtonHeight,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed to move next button!";
		}

		Invalidate();
	}

	void MainWindow::OnErase() const
	{
		RECT clientArea = { 0 };

		if (!GetWindowRect(_window, &clientArea))
		{
			LOGD << L"GetWindowRect failed!";
			return;
		}

		const Gdiplus::Rect area(0, 0, clientArea.right - clientArea.left, clientArea.bottom - clientArea.top);
		GdiExtensions::ContextWrapper context(_window);
		const Gdiplus::SolidBrush grayBrush(Gdiplus::Color::LightGray);
		context.Graphics().FillRectangle(&grayBrush, area);
	}

	void MainWindow::OnPaint()
	{
		GdiExtensions::ContextWrapper context(_canvas);

		if (!context.IsValid())
		{
			return;
		}

		Gdiplus::Bitmap bitmap(_canvasArea.Width, _canvasArea.Height);
		Gdiplus::Graphics buffer(&bitmap);

		const Gdiplus::SolidBrush grayBrush(Gdiplus::Color::DarkGray);
		buffer.FillRectangle(&grayBrush, 0, 0, _canvasArea.Width, _canvasArea.Height);

		Gdiplus::Image* image = _imageCache->Current();

		if (image)
		{
			Gdiplus::SizeF size(Gdiplus::REAL(image->GetWidth()), Gdiplus::REAL(image->GetHeight()));
			Gdiplus::Rect scaled;

			GdiExtensions::ScaleAndCenterTo(_canvasArea, size, scaled);
			GdiExtensions::Zoom(scaled, _zoomPercent);
			scaled.Offset(_mouseHandler->MouseDragOffset());

			if (_mouseHandler->IsDragging())
			{
				const Gdiplus::Pen pen(Gdiplus::Color::Gray, 2.0f);
				buffer.DrawRectangle(&pen, scaled);
			}
			else
			{
				buffer.DrawImage(image, scaled);
			}
		}

		context.Graphics().DrawImage(&bitmap, 0, 0, _canvasArea.Width, _canvasArea.Height);
	}

	void MainWindow::OnImageChanged(std::filesystem::path path)
	{
		_zoomPercent = 0;
		_mouseHandler->ResetOffsets();

		Invalidate();

		const std::wstring title = L"Picture Browser 2.2 - " + path.filename().wstring();
		SetWindowText(_window, title.c_str());
	}

	void MainWindow::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (_zoomPercent > 0)
				{
					_zoomPercent -= 5;
					Invalidate();
				}
				break;
			case VK_OEM_PLUS:
				if (_zoomPercent < 1000)
				{
					_zoomPercent += 5;
					Invalidate();
				}
				break;
		}

		LOGD << _zoomPercent;
	}

	void MainWindow::OnCommand(WPARAM wParam)
	{
		switch (LOWORD(wParam))
		{
			case IDC_ZOOM_OUT_BUTTON:
			{
				// TODO: disable button if no image is loaded
				OnZoom(VK_OEM_MINUS);
				break;
			}
			case IDC_ZOOM_IN_BUTTON:
			{
				// TODO: disable button if no image is loaded
				OnZoom(VK_OEM_PLUS);
				break;
			}
			case IDM_EXIT:
			{
				DestroyWindow(_window);
				break;
			}
			case IDM_ABOUT:
			{
				DialogBox(Instance(), MAKEINTRESOURCE(IDD_ABOUT), _window, GenericOkDialog);
				break;
			}
			case IDM_KEYBOARD:
			{
				DialogBox(Instance(), MAKEINTRESOURCE(IDD_KEYBOARD), _window, GenericOkDialog);
				break;
			}
			case IDM_OPTIONS_USE_CACHING:
			{
				const UINT checkedState = CheckedState(IDM_OPTIONS_USE_CACHING);

				if (checkedState == MFS_CHECKED)
				{
					Registry::Set(L"Software\\PictureBrowser\\UseCaching", false);
					SetCheckedState(IDM_OPTIONS_USE_CACHING, MFS_UNCHECKED);
				}
				else
				{
					Registry::Set(L"Software\\PictureBrowser\\UseCaching", true);
					SetCheckedState(IDM_OPTIONS_USE_CACHING, MFS_CHECKED);
				}

				break;
			}
		}
	}
	void MainWindow::Invalidate(bool erase)
	{
		const RECT canvasArea =
		{
			_canvasArea.GetLeft(),
			_canvasArea.GetTop(),
			_canvasArea.GetRight(),
			_canvasArea.GetBottom()
		};

		if (!InvalidateRect(_window, &canvasArea, erase))
		{
			LOGD << L"InvalidateRect failed!";
			return;
		}

		LOGD << canvasArea;
	}

	UINT MainWindow::CheckedState(UINT menuEntry) const
	{
		const HMENU menu = GetMenu(_window);
		MENUITEMINFO menuItemInfo = { 0 };
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_STATE;

		if (!GetMenuItemInfo(menu, IDM_OPTIONS_USE_CACHING, FALSE, &menuItemInfo))
		{
			LOGD << L"GetMenuItemInfo failed!";
			return MFS_DEFAULT; // Maybe not the best idea...
		}

		return menuItemInfo.fState;
	}

	void MainWindow::SetCheckedState(UINT menuEntry, UINT state) const
	{
		const HMENU menu = GetMenu(_window);
		MENUITEMINFO menuItemInfo = { 0 };
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_STATE;
		menuItemInfo.fState = state;

		if (!SetMenuItemInfo(menu, IDM_OPTIONS_USE_CACHING, FALSE, &menuItemInfo))
		{
			LOGD << L"SetMenuItemInfo failed!";
		}
	}

	INT_PTR CALLBACK MainWindow::GenericOkDialog(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);

		switch (message)
		{
			case WM_INITDIALOG:
			{
				InvalidateRect(dialog, nullptr, true);
				return static_cast<INT_PTR>(TRUE);
			}
			case WM_COMMAND:
			{
				if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
				{
					EndDialog(dialog, LOWORD(wParam));
					return static_cast<INT_PTR>(TRUE);
				}

				break;
			}
		}

		return static_cast<INT_PTR>(FALSE);
	}
}