#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"
#include "Registry.hpp"

namespace PictureBrowser
{
	MainWindow* _mainWindow = nullptr;

	constexpr UINT Padding = 5;
	constexpr UINT ButtonWidth = 50;
	constexpr UINT ButtonHeight = 25;
	constexpr UINT FileListWidth = 250;

	MainWindow::MainWindow()
	{
		_mainWindow = this;
	}

	MainWindow::~MainWindow()
	{
		_mainWindow = nullptr;
	}

	ATOM MainWindow::Register() const
	{
		WNDCLASSEXW windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		windowClass.lpfnWndProc = WindowProcedure;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = _instance;
		windowClass.hIcon = LoadIcon(_instance, MAKEINTRESOURCE(IDI_PICTURE_BROWSER));
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_MENU);
		windowClass.lpszClassName = L"PictureBrowser";
		windowClass.hIconSm = LoadIcon(_instance, MAKEINTRESOURCE(IDI_PICTURE_BROWSER));

		return RegisterClassEx(&windowClass);
	}

	bool MainWindow::InitInstance(HINSTANCE instance, int showCommand)
	{
		_instance = instance;

		if (!Register())
		{
			LOGD << L"Failed to register main window!";
			return false;
		}

		_window = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			L"PictureBrowser",
			L"Picture Browser 2.2",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			800,
			800,
			nullptr,
			nullptr,
			_instance,
			nullptr);

		if (!_window)
		{
			LOGD << L"Failed to create main window!";
			return false;
		}

		if (!ShowWindow(_window, showCommand))
		{
			LOGD << L"Failed to show window!";
		}

		if (!UpdateWindow(_window))
		{
			LOGD << L"Failed to update window!";
		}

		OnResize();

		// SetWindowTheme(_window, L" ", L" "); // This will make even worse looks

		return true;
	}

	void MainWindow::Open(const std::filesystem::path& path)
	{
		_fileListHandler->Open(path);
	}

	void MainWindow::RecalculatePaintArea(HWND window)
	{
		RECT clientArea = { };

		if (!GetClientRect(window, &clientArea))
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

	void MainWindow::OnCreate(HWND window)
	{
		RecalculatePaintArea(window);

		_fileListBox = CreateWindow(
			WC_LISTBOX,
			L"Filelist...",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
			_fileListArea.X,
			_fileListArea.Y,
			_fileListArea.Width,
			_fileListArea.Height,
			window,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			_instance,
			nullptr);

		_canvas = CreateWindow(
			WC_STATIC,
			nullptr,
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_canvasArea.X,
			_canvasArea.Y,
			_canvasArea.Width,
			_canvasArea.Height,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			_instance,
			nullptr);

		_zoomOutButton = CreateWindow(
			WC_BUTTON,
			L"-",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.X,
			_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			_instance,
			nullptr);

		_zoomInButton = CreateWindow(
			WC_BUTTON,
			L"+",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.Width - ButtonWidth,
			_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_IN_BUTTON),
			_instance,
			nullptr);

		_previousPictureButton = CreateWindow(
			WC_BUTTON,
			L"<",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.X,
			_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON),
			_instance,
			nullptr);

		_nextPictureButton = CreateWindow(
			WC_BUTTON,
			L">",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			_mainArea.Width - ButtonWidth,
			_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON),
			_instance,
			nullptr);

		const bool useCaching = Registry::Get(L"Software\\PictureBrowser\\UseCaching", true);
		SetCheckedState(IDM_OPTIONS_USE_CACHING, useCaching ? MFS_CHECKED : MFS_UNCHECKED);

		_imageCache = std::make_shared<ImageCache>(useCaching);

		_fileListHandler = std::make_unique<FileListHandler>(
			window,
			_fileListBox,
			_imageCache,
			std::bind(&MainWindow::OnImageChanged, this, std::placeholders::_1));
		
		_mouseHandler = std::make_unique<MouseHandler>(
			window,
			_canvas,
			std::bind(&MainWindow::Invalidate, this, true));

		_keyboardHandler = std::make_unique<KeyboardHandler>(
			window,
			_fileListBox,
			std::bind(&FileListHandler::SelectImage, _fileListHandler.get(), std::placeholders::_1));
	}

	void MainWindow::OnResize()
	{
		RecalculatePaintArea(_window);

		if (!SetWindowPos(
			_fileListBox,
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

	void MainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
	{
		if (reinterpret_cast<HWND>(wParam) == _mainWindow->_fileListBox)
		{
			_mainWindow->_fileListHandler->OnContextMenu(lParam);
		}
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
			case IDC_PREV_BUTTON:
			{
				// TODO: disable button if no image is loaded
				_keyboardHandler->OnKeyUp(VK_LEFT);
				break;
			}
			case IDC_NEXT_BUTTON:
			{
				// TODO: disable button if no image is loaded
				_keyboardHandler->OnKeyUp(VK_RIGHT);
				break;
			}
			case IDC_POPUP_COPY:
			{
				_fileListHandler->OnPopupClosed();
				break;
			}
			case IDM_EXIT:
			{
				DestroyWindow(_window);
				break;
			}
			case IDM_ABOUT:
			{
				DialogBox(_mainWindow->_instance, MAKEINTRESOURCE(IDD_ABOUT), _window, GenericOkDialog);
				break;
			}
			case IDM_KEYBOARD:
			{
				DialogBox(_mainWindow->_instance, MAKEINTRESOURCE(IDD_KEYBOARD), _window, GenericOkDialog);
				break;
			}
			case IDM_OPEN:
			{
				_fileListHandler->OnOpenMenu();
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

		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
			{
				_fileListHandler->OnSelectionChanged();
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

	LRESULT CALLBACK MainWindow::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_CREATE:
			{
				_mainWindow->OnCreate(window);
				break;
			}
			case WM_DESTROY:
			{
				_mainWindow->_fileListHandler->Clear();
				PostQuitMessage(0);
				break;
			}
			case WM_SIZE:
			{
				_mainWindow->OnResize();
				break;
			}
			case WM_PAINT:
			{
				DefWindowProc(window, message, wParam, lParam);
				_mainWindow->OnPaint();
				break;
			}
			case WM_ERASEBKGND:
			{
				_mainWindow->OnErase();
				break;
			}
			case WM_CONTEXTMENU:
			{
				_mainWindow->OnContextMenu(wParam, lParam);
				break;
			}
			case WM_KEYUP:
			{
				_mainWindow->_keyboardHandler->OnKeyUp(wParam);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				_mainWindow->_mouseHandler->OnDoubleClick();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				_mainWindow->_mouseHandler->OnLeftMouseDown(lParam);
				break;
			}
			case WM_MOUSEMOVE:
			{
				_mainWindow->_mouseHandler->OnMouseMove(lParam);
				break;
			}
			case WM_LBUTTONUP:
			{
				_mainWindow->_mouseHandler->OnLeftMouseUp(lParam);
				break;
			}
			case WM_COMMAND:
			{
				_mainWindow->OnCommand(wParam);
				break;
			}
			case WM_DROPFILES:
			{
				_mainWindow->_fileListHandler->OnFileDrop(wParam);
				break;
			}
			default:
			{
				return DefWindowProc(window, message, wParam, lParam);
			}
		}

		return EXIT_SUCCESS;
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