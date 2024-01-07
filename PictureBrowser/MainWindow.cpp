#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"
#include "LogWrap.hpp"
#include "Registry.hpp"

namespace PictureBrowser
{
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

	bool MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM)
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
			case WM_COMMAND:
			{
				OnCommand(wParam);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				OnDoubleClick();
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
			std::unreachable();
		}

		clientArea.left += Padding;
		clientArea.top += Padding;
		clientArea.right -= Padding;
		clientArea.bottom -= Padding;

		_fileListArea.left = clientArea.left;
		_fileListArea.top = clientArea.top;
		_fileListArea.right = FileListWidth;
		_fileListArea.bottom = clientArea.bottom;

		_mainArea.left = _fileListArea.right + (Padding * 2);
		_mainArea.top = clientArea.top;
		_mainArea.right = clientArea.right;
		_mainArea.bottom = clientArea.bottom;

		_canvasArea.left = _mainArea.left;
		_canvasArea.top = _mainArea.top + ButtonHeight + Padding;
		_canvasArea.right = _mainArea.right - FileListWidth - (Padding * 2);
		_canvasArea.bottom = _mainArea.bottom - (ButtonHeight * 2) - (Padding * 3);

		LOGD << L"File list area: " << _fileListArea;
		LOGD << L"Main area: " << _mainArea;
		LOGD << L"Canvas area: " << _canvasArea;
	}

	void MainWindow::OnCreate()
	{
		RecalculatePaintArea();

		_zoomOutButton = AddWidget(
			WC_BUTTON,
			L"-",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.left,
			_mainArea.top,
			ButtonWidth,
			ButtonHeight,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON));

		_zoomInButton = AddWidget(
			WC_BUTTON,
			L"+",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.right,
			_mainArea.top,
			ButtonWidth,
			ButtonHeight,
			reinterpret_cast<HMENU>(IDC_ZOOM_IN_BUTTON));

		_previousPictureButton = AddWidget(
			WC_BUTTON,
			L"<",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.left,
			_mainArea.bottom,
			ButtonWidth,
			ButtonHeight,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON));

		_nextPictureButton = AddWidget(
			WC_BUTTON,
			L">",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.right,
			_mainArea.bottom,
			ButtonWidth,
			ButtonHeight,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON));

		const bool useCaching = Registry::Get(L"Software\\PictureBrowser\\UseCaching", true);
		SetCheckedState(IDM_OPTIONS_USE_CACHING, useCaching ? MFS_CHECKED : MFS_UNCHECKED);

		_imageCache = std::make_shared<ImageCache>(useCaching);

		_canvasWidget = std::make_unique<CanvasWidget>(
			Instance(),
			_window, 
			_imageCache);

		_canvasWidget->Intercept(_window);

		_fileListWidget = std::make_unique<FileListWidget>(
			Instance(),
			_window,
			_imageCache,
			std::bind(&CanvasWidget::OnImageChanged, _canvasWidget.get(), std::placeholders::_1));

		_fileListWidget->Intercept(_window);
	}

	void MainWindow::OnResize()
	{
		RecalculatePaintArea();

		if (!_fileListWidget->SetPosition(
			HWND_TOP,
			_fileListArea.left,
			_fileListArea.top,
			_fileListArea.right,
			_fileListArea.bottom,
			SWP_NOMOVE | SWP_NOZORDER))
		{
			std::unreachable();
		}

		if (!_canvasWidget->SetPosition(
			HWND_TOP,
			_canvasArea.left,
			_canvasArea.top,
			_canvasArea.right,
			_canvasArea.bottom,
			SWP_NOZORDER))
		{
			std::unreachable();
		}

		if (!_zoomOutButton.SetPosition(
			HWND_TOP,
			_mainArea.left,
			_mainArea.top,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			std::unreachable();
		}

		if (!_zoomInButton.SetPosition(
			HWND_TOP,
			_mainArea.right - ButtonWidth,
			_mainArea.top,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			std::unreachable();
		}

		if (!_previousPictureButton.SetPosition(
			HWND_TOP,
			_mainArea.left,
			_mainArea.bottom - ButtonHeight,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			std::unreachable();
		}

		if (!_nextPictureButton.SetPosition(
			HWND_TOP,
			_mainArea.right - ButtonWidth,
			_mainArea.bottom - ButtonHeight,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			std::unreachable();
		}

		_canvasWidget->Resize();

		if (!InvalidateRect(_window, &_canvasArea, false))
		{
			std::unreachable();
		}
	}

	void MainWindow::OnCommand(WPARAM wParam)
	{
		switch (LOWORD(wParam))
		{
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

	void MainWindow::OnDoubleClick()
	{
		WINDOWPLACEMENT placement = {};

		if (!GetWindowPlacement(_window, &placement))
		{
			std::unreachable();
		}

		const UINT show = placement.showCmd == SW_NORMAL ? SW_SHOWMAXIMIZED : SW_NORMAL;

		Show(show);
	}

	UINT MainWindow::CheckedState(UINT menuEntry) const
	{
		const HMENU menu = GetMenu(_window);
		MENUITEMINFO menuItemInfo;
		ZeroInit(&menuItemInfo);
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_STATE;

		if (!GetMenuItemInfo(menu, menuEntry, FALSE, &menuItemInfo))
		{
			std::unreachable();
		}

		return menuItemInfo.fState;
	}

	void MainWindow::SetCheckedState(UINT menuEntry, UINT state) const
	{
		const HMENU menu = GetMenu(_window);
		MENUITEMINFO menuItemInfo;
		ZeroInit(&menuItemInfo);
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_STATE;
		menuItemInfo.fState = state;

		if (!SetMenuItemInfo(menu, menuEntry, FALSE, &menuItemInfo))
		{
			std::unreachable();
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