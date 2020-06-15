#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"

namespace PictureBrowser
{
	MainWindow* g_mainWindow = nullptr;

	constexpr UINT Padding = 5;
	constexpr UINT ButtonWidth = 50;
	constexpr UINT ButtonHeight = 25;
	constexpr UINT FileListWidth = 250;

	std::wstring LoadStdString(HINSTANCE instance, UINT id)
	{
		std::wstring buffer(1, L'\0');
		const int length = LoadString(instance, id, &buffer.front(), 0);

		if (length <= 0)
		{
			LOGD << L"Failed to load string ID: " << id;
			return {};
		}

		buffer.resize(static_cast<size_t>(length));

		if (LoadString(instance, id, &buffer.front(), length + 1) != length)
		{
			return {};
		}

		return buffer;
	}

	MainWindow::MainWindow()
	{
		g_mainWindow = this;
	}

	MainWindow::~MainWindow()
	{
		g_mainWindow = nullptr;
	}

	bool MainWindow::LoadStrings()
	{
		m_title = LoadStdString(m_instance, IDS_PICTURE_BROWSER);
		m_windowClassName = LoadStdString(m_instance, IDC_PICTURE_BROWSER);

		return !(m_title.empty() || m_windowClassName.empty());
	}

	ATOM MainWindow::Register() const
	{
		WNDCLASSEXW windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		windowClass.lpfnWndProc = WindowProcedure;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = m_instance;
		windowClass.hIcon = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_PICTURE_BROWSER));
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_MENU);
		windowClass.lpszClassName = m_windowClassName.c_str();
		windowClass.hIconSm = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_PICTURE_BROWSER));

		return RegisterClassEx(&windowClass);
	}

	bool MainWindow::InitInstance(HINSTANCE instance, int showCommand)
	{
		m_instance = instance;

		if (!LoadStrings())
		{
			LOGD << L"Failed to load strings!";
			return false;
		}

		if (!Register())
		{
			LOGD << L"Failed to register main window!";
			return false;
		}

		m_window = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			m_windowClassName.c_str(),
			m_title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			800,
			800,
			nullptr,
			nullptr,
			m_instance,
			nullptr);

		if (!m_window)
		{
			LOGD << L"Failed to create main window!";
			return false;
		}

		if (!ShowWindow(m_window, showCommand))
		{
			LOGD << L"Failed to show window!";
		}

		if (!UpdateWindow(m_window))
		{
			LOGD << L"Failed to update window!";
		}

		OnResize();

		// SetWindowTheme(m_window, L" ", L" "); // This will make even worse looks

		return true;
	}

	void MainWindow::Open(const std::filesystem::path& path)
	{
		m_fileListHandler->Open(path);
	}

	void MainWindow::RecalculatePaintArea(HWND window)
	{
		RECT clientArea = { 0 };

		if (!GetClientRect(window, &clientArea))
		{
			LOGD << L"GetClientRect failed!";
			clientArea.top = 0;
			clientArea.left = 0;
			clientArea.right = 800;
			clientArea.bottom = 600;
		}

		m_fileListArea.X = clientArea.left + Padding;
		m_fileListArea.Y = clientArea.top + Padding;
		m_fileListArea.Width = FileListWidth;
		m_fileListArea.Height = clientArea.bottom - Padding;

		m_mainArea.X = m_fileListArea.GetRight() + Padding;
		m_mainArea.Y = clientArea.top + Padding;
		m_mainArea.Width = clientArea.right - m_fileListArea.GetRight() - Padding * 2;
		m_mainArea.Height = clientArea.bottom - Padding * 2;

		m_canvasArea.X = m_mainArea.X;
		m_canvasArea.Y = m_mainArea.Y + Padding + ButtonHeight;
		m_canvasArea.Width = m_mainArea.Width;
		m_canvasArea.Height = m_mainArea.Height - Padding * 2 - ButtonHeight * 2;

		LOGD << L"m_fileListArea: " << m_fileListArea;
		LOGD << L"m_mainArea: " << m_mainArea;
		LOGD << L"m_canvasArea: " << m_canvasArea;
	}

	void MainWindow::OnCreate(HWND window)
	{
		RecalculatePaintArea(window);

		m_fileListBox = CreateWindow(
			WC_LISTBOX,
			L"Filelist...",
			WS_VISIBLE | WS_CHILD | LBS_STANDARD,
			m_fileListArea.X,
			m_fileListArea.Y,
			m_fileListArea.Width,
			m_fileListArea.Height,
			window,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			m_instance,
			nullptr);

		m_canvas = CreateWindow(
			WC_STATIC,
			nullptr,
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_canvasArea.X,
			m_canvasArea.Y,
			m_canvasArea.Width,
			m_canvasArea.Height,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			m_instance,
			nullptr);

		m_zoomOutButton = CreateWindow(
			WC_BUTTON,
			L"-",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_mainArea.X,
			m_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_OUT_BUTTON),
			m_instance,
			nullptr);

		m_zoomInButton = CreateWindow(
			WC_BUTTON,
			L"+",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_mainArea.Width - ButtonWidth,
			m_mainArea.Y,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_ZOOM_IN_BUTTON),
			m_instance,
			nullptr);

		m_previousPictureButton = CreateWindow(
			WC_BUTTON,
			L"<",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_mainArea.X,
			m_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON),
			m_instance,
			nullptr);

		m_nextPictureButton = CreateWindow(
			WC_BUTTON,
			L">",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_mainArea.Width - ButtonWidth,
			m_mainArea.Height - ButtonHeight,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON),
			m_instance,
			nullptr);

		m_imageCache = std::make_shared<ImageCache>();
		m_fileListHandler = std::make_unique<FileListHandler>(window, m_fileListBox, m_imageCache, std::bind(&MainWindow::OnImageChanged, this, std::placeholders::_1));
		m_mouseHandler = std::make_unique<MouseHandler>(window, m_canvas, std::bind(&MainWindow::Invalidate, this, true));
		m_keyboardHandler = std::make_unique<KeyboardHandler>(window, m_fileListBox, std::bind(&FileListHandler::SelectImage, m_fileListHandler.get(), std::placeholders::_1));
	}

	void MainWindow::OnResize()
	{
		RecalculatePaintArea(m_window);

		if (!SetWindowPos(
			m_fileListBox,
			HWND_TOP,
			0,
			0,
			m_fileListArea.Width,
			m_fileListArea.Height,
			SWP_NOMOVE | SWP_NOZORDER))
		{
			LOGD << L"Failed to move file list!";
		}

		if (!SetWindowPos(
			m_canvas,
			HWND_TOP,
			m_canvasArea.GetLeft(),
			m_canvasArea.GetTop(),
			m_canvasArea.Width,
			m_canvasArea.Height,
			SWP_NOZORDER))
		{
			LOGD << L"Failed move minus button!";
		}

		if (!SetWindowPos(
			m_zoomOutButton,
			HWND_TOP,
			m_mainArea.GetLeft(),
			m_mainArea.GetTop(),
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move minus button!";
		}

		if (!SetWindowPos(
			m_zoomInButton,
			HWND_TOP,
			m_mainArea.GetRight() - ButtonWidth,
			m_mainArea.GetTop(),
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move plus button!";
		}

		if (!SetWindowPos(
			m_previousPictureButton,
			HWND_TOP,
			m_mainArea.GetLeft(),
			m_mainArea.GetBottom() - ButtonHeight,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move previous button!";
		}

		if (!SetWindowPos(
			m_nextPictureButton,
			HWND_TOP,
			m_mainArea.GetRight() - ButtonWidth,
			m_mainArea.GetBottom() - ButtonHeight,
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

		if (!GetWindowRect(m_window, &clientArea))
		{
			LOGD << L"GetWindowRect failed!";
			return;
		}

		const Gdiplus::Rect area(0, 0, clientArea.right - clientArea.left, clientArea.bottom - clientArea.top);
		GdiExtensions::ContextWrapper context(m_window);
		const Gdiplus::SolidBrush grayBrush(Gdiplus::Color::LightGray);
		context.Graphics().FillRectangle(&grayBrush, area);
	}

	void MainWindow::OnPaint()
	{
		GdiExtensions::ContextWrapper context(m_canvas);

		if (!context.IsValid())
		{
			return;
		}

		Gdiplus::Bitmap bitmap(m_canvasArea.Width, m_canvasArea.Height);
		Gdiplus::Graphics buffer(&bitmap);

		const Gdiplus::SolidBrush grayBrush(Gdiplus::Color::DarkGray);
		buffer.FillRectangle(&grayBrush, 0, 0, m_canvasArea.Width, m_canvasArea.Height);

		Gdiplus::Image* image = m_imageCache->Current();

		if (image)
		{
			Gdiplus::SizeF size(Gdiplus::REAL(image->GetWidth()), Gdiplus::REAL(image->GetHeight()));
			Gdiplus::Rect scaled;

			GdiExtensions::ScaleAndCenterTo(m_canvasArea, size, scaled);
			GdiExtensions::Zoom(scaled, m_zoomPercent);
			scaled.Offset(m_mouseHandler->MouseDragOffset());

			if (m_mouseHandler->IsDragging())
			{
				const Gdiplus::Pen pen(Gdiplus::Color::Gray, 2.0f);
				buffer.DrawRectangle(&pen, scaled);
			}
			else
			{
				buffer.DrawImage(image, scaled);
			}
		}

		context.Graphics().DrawImage(&bitmap, 0, 0, m_canvasArea.Width, m_canvasArea.Height);
	}

	void MainWindow::OnImageChanged(std::filesystem::path path)
	{
		m_zoomPercent = 0;
		m_mouseHandler->ResetOffsets();

		Invalidate();

		const std::wstring title = m_title + L" - " + path.filename().wstring();
		SetWindowText(m_window, title.c_str());
	}

	void MainWindow::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (m_zoomPercent > 0)
				{
					m_zoomPercent -= 5;
					Invalidate();
				}
				break;
			case VK_OEM_PLUS:
				if (m_zoomPercent < 1000)
				{
					m_zoomPercent += 5;
					Invalidate();
				}
				break;
		}

		LOGD << m_zoomPercent;
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
				m_keyboardHandler->OnKeyUp(VK_LEFT);
				break;
			}
			case IDC_NEXT_BUTTON:
			{
				// TODO: disable button if no image is loaded
				m_keyboardHandler->OnKeyUp(VK_RIGHT);
				break;
			}
			case IDM_EXIT:
			{
				DestroyWindow(m_window);
				break;
			}
			case IDM_ABOUT:
			{
				DialogBox(g_mainWindow->m_instance, MAKEINTRESOURCE(IDD_ABOUT), m_window, GenericOkDialog);
				break;
			}
			case IDM_KEYBOARD:
			{
				DialogBox(g_mainWindow->m_instance, MAKEINTRESOURCE(IDD_KEYBOARD), m_window, GenericOkDialog);
				break;
			}
			case IDM_OPEN:
			{
				m_fileListHandler->OnOpenMenu();
				break;
			}
		}

		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
			{
				m_fileListHandler->OnSelectionChanged();
				break;
			}
		}
	}
	void MainWindow::Invalidate(bool erase)
	{
		const RECT canvasArea =
		{
			m_canvasArea.GetLeft(),
			m_canvasArea.GetTop(),
			m_canvasArea.GetRight(),
			m_canvasArea.GetBottom()
		};

		if (!InvalidateRect(m_window, &canvasArea, erase))
		{
			LOGD << L"InvalidateRect failed!";
			return;
		}

		LOGD << canvasArea;
	}

	LRESULT CALLBACK MainWindow::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_CREATE:
			{
				g_mainWindow->OnCreate(window);
				break;
			}
			case WM_DESTROY:
			{
				g_mainWindow->m_fileListHandler->Clear();
				PostQuitMessage(0);
				break;
			}
			case WM_SIZE:
			{
				g_mainWindow->OnResize();
				break;
			}
			case WM_ERASEBKGND:
			{
				g_mainWindow->OnErase();
				break;
			}
			case WM_PAINT:
			{
				DefWindowProc(window, message, wParam, lParam);
				g_mainWindow->OnPaint();
				break;
			}
			case WM_KEYUP:
			{
				g_mainWindow->m_keyboardHandler->OnKeyUp(wParam);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				g_mainWindow->m_mouseHandler->OnDoubleClick();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				g_mainWindow->m_mouseHandler->OnLeftMouseDown(lParam);
				break;
			}
			case WM_MOUSEMOVE:
			{
				g_mainWindow->m_mouseHandler->OnMouseMove(lParam);
				break;
			}
			case WM_LBUTTONUP:
			{
				g_mainWindow->m_mouseHandler->OnLeftMouseUp(lParam);
				break;
			}
			case WM_COMMAND:
			{
				g_mainWindow->OnCommand(wParam);
				break;
			}
			case WM_DROPFILES:
			{
				g_mainWindow->m_fileListHandler->OnFileDrop(wParam);
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