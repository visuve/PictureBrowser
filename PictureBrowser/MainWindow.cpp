#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"

namespace PictureBrowser
{
	MainWindow* g_mainWindow = nullptr;

	constexpr UINT ButtonWidth = 50;
	constexpr UINT ButtonHeight = 25;
	constexpr UINT FileListWidth = 250;

	std::wstring LoadStdString(HINSTANCE instance, UINT id)
	{
		std::wstring buffer(1, L'\0');
		int length = LoadString(instance, id, &buffer.front(), 0);

		if (length)
		{
			buffer.resize(length);
			LoadString(instance, id, &buffer.front(), length + 1);
		}

		return buffer;
	}

	Gdiplus::Rect Adjust(RECT canvasSize, float imageWidth, float imageHeight)
	{
		float canvasWidth = static_cast<float>(canvasSize.right - canvasSize.left);
		float canvasHeight = static_cast<float>(canvasSize.bottom - canvasSize.top);

		float aspectRatio = min(
			canvasWidth / imageWidth,
			canvasHeight / imageHeight);

		UINT x = UINT(canvasWidth - imageWidth * aspectRatio) / 2;
		UINT y = UINT(canvasHeight - imageHeight * aspectRatio) / 2;
		UINT w = UINT(imageWidth * aspectRatio);
		UINT h = UINT(imageHeight * aspectRatio);

		return Gdiplus::Rect(x + canvasSize.left, y + canvasSize.top, w, h);
	}

	Gdiplus::Rect Adjust(RECT canvasSize, UINT imageWidth, UINT imageHeight)
	{
		return Adjust(canvasSize, float(imageWidth), float(imageHeight));
	}

	MainWindow::MainWindow()
	{
		g_mainWindow = this;
	}

	MainWindow::~MainWindow()
	{
		if (g_mainWindow)
		{
			UnregisterClass(m_windowClassName.c_str(), m_instance);
			g_mainWindow = nullptr;
		}
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
			return false;
		}

		if (!Register())
		{
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
			return false;
		}

		ShowWindow(m_window, showCommand);
		UpdateWindow(m_window);
		OnResize();

		// SetWindowTheme(m_window, L" ", L" "); // This will make even worse looks

		return true;
	}

	bool MainWindow::LoadFileList(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			const std::wstring message = 
				L"The file you have entered does not appear to exist:\n" + path.wstring();

			MessageBox(m_window,
				message.c_str(),
				L"File not found!",
				MB_OK | MB_ICONINFORMATION);

			return false;
		}

		if (_wcsicmp(path.extension().c_str(), L".jpg") != 0)
		{
			MessageBox(m_window,
				L"Only the picture format of the future is supported.",
				L"Unsupported file format!",
				MB_OK | MB_ICONINFORMATION);

			return false;
		}

		std::wstring filter = path.parent_path().wstring() + L"\\*.jpg";
		bool list = DlgDirList(m_window, &filter.front(), IDC_LISTBOX, 0, DDL_READWRITE);
		bool send = SendMessage(m_fileListBox, LB_SELECTSTRING, 0, reinterpret_cast<LPARAM>(path.filename().c_str()));
		return list && send;
	}

	void MainWindow::ShowImage(const std::filesystem::path& path)
	{
		m_image.reset(Gdiplus::Image::FromFile(path.c_str()));

		if (m_image)
		{
			InvalidateRect(m_window, nullptr, true);

			std::wstring title = m_title + L" - " + path.filename().wstring();
			SetWindowText(m_window, title.c_str());
		}
	}

	void MainWindow::OnCreate(HWND window)
	{
		m_prevButton = CreateWindow(
			WC_BUTTON,
			L"<",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			300,
			710,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON),
			m_instance,
			nullptr);

		m_nextButton = CreateWindow(
			WC_BUTTON,
			L">",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			450,
			710,
			ButtonWidth,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON),
			m_instance,
			nullptr);

		m_fileListBox = CreateWindow(
			WC_LISTBOX,
			L"Filelist...",
			WS_VISIBLE | WS_CHILD | WS_EX_NOACTIVATE,
			0,
			0,
			FileListWidth,
			800,
			window,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			m_instance,
			nullptr);
	}

	void MainWindow::OnResize()
	{
		RECT canvasSize = { 0 };

		if (GetClientRect(m_window, &canvasSize))
		{
			std::swap(m_canvasSize, canvasSize);
			m_canvasSize.left += FileListWidth;

			SetWindowPos(
				m_prevButton,
				HWND_TOP,
				m_canvasSize.left,
				m_canvasSize.bottom - ButtonHeight,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);

			SetWindowPos(
				m_nextButton,
				HWND_TOP,
				m_canvasSize.right - ButtonWidth,
				m_canvasSize.bottom - ButtonHeight,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);

			SetWindowPos(
				m_fileListBox,
				HWND_TOP,
				0,
				0,
				FileListWidth,
				m_canvasSize.bottom, // TODO: there remains some weird gap
				SWP_NOMOVE | SWP_NOZORDER);
		}
	}

	void MainWindow::OnPaint() const
	{
		PAINTSTRUCT ps = { 0 };
		HDC hdc = BeginPaint(m_window, &ps);

		if (m_image)
		{
			Gdiplus::Graphics graphics(hdc);

			Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color::DarkGray);
			graphics.FillRectangle(&mySolidBrush, 0, 0, m_canvasSize.right, m_canvasSize.bottom);

			Gdiplus::Rect rect = Adjust(m_canvasSize, m_image->GetWidth(), m_image->GetHeight());
			graphics.DrawImage(m_image.get(), rect);
		}

		EndPaint(m_window, &ps);
	}

	void MainWindow::OnDoubleClick()
	{
		if (ShowWindow(m_window, m_maximized ? SW_NORMAL : SW_SHOWMAXIMIZED))
		{
			m_maximized = !m_maximized;
		}
	}

	void MainWindow::OnKeyUp(WPARAM wParam)
	{
		LONG_PTR count = SendMessage(m_fileListBox, LB_GETCOUNT, 0, 0);

		if (!count)
		{
			MessageBox(
				m_window,
				L"Please drag & drop a file or open from the menu!",
				L"No image selected!",
				MB_OK | MB_ICONINFORMATION);

			return;
		}

		switch (wParam)
		{
			case VK_LEFT:
			{
				LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

				if (current > 0)
				{
					ChangeSelection(--current);
				}

				return;
			}
			case VK_RIGHT:
			{
				LONG_PTR lastIndex = count -1;
				LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

				if (current < lastIndex)
				{
					ChangeSelection(++current);
				}

				return;
			}
		}
	}

	void MainWindow::OnCommand(WPARAM wParam)
	{
		switch (LOWORD(wParam))
		{
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
				OnOpenMenu();
				break;
			}
			case IDC_PREV_BUTTON:
			{
				OnKeyUp(VK_LEFT);
				break;
			}
			case IDC_NEXT_BUTTON:
			{
				OnKeyUp(VK_RIGHT);
				break;
			}
		}
	}

	void MainWindow::OnFileDrop(WPARAM wParam)
	{
		HDROP dropInfo = reinterpret_cast<HDROP>(wParam);
		UINT required = DragQueryFile(dropInfo, 0, nullptr, 0);
		std::wstring path(required, L'\0');

		if (required)
		{
			path.resize(required);
			UINT result = DragQueryFile(dropInfo, 0, &path.front(), required + 1); // null terminator can be overwritten
			path.resize(result);
		}

		if (!path.empty() && LoadFileList(path))
		{
			ShowImage(path);
		}

		DragFinish(dropInfo);
	}

	void MainWindow::OnOpenMenu()
	{
		OPENFILENAME openFile = { 0 };
		wchar_t filePath[0x1000] = { 0 };

		openFile.lStructSize = sizeof(openFile);
		openFile.hwndOwner = m_window;
		openFile.lpstrFile = filePath;
		openFile.nMaxFile = 0xFFF;
		openFile.lpstrFilter = L"Picture format of the future (*.jpg)\0*.jpg\0";
		openFile.nFilterIndex = 1;
		openFile.lpstrFileTitle = nullptr;
		openFile.nMaxFileTitle = 0;
		openFile.lpstrInitialDir = nullptr;
		openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&openFile) && LoadFileList(openFile.lpstrFile))
		{
			ShowImage(openFile.lpstrFile);
		}
	}

	void MainWindow::ChangeSelection(LONG_PTR current)
	{
		if (SendMessage(m_fileListBox, LB_SETCURSEL, current, 0) < 0)
		{
			return;
		}

		size_t length = static_cast<size_t>(SendMessage(m_fileListBox, LB_GETTEXTLEN, current, 0));
		std::wstring buffer(length, '\0');

		if (SendMessage(m_fileListBox, LB_GETTEXT, current, reinterpret_cast<LPARAM>(&buffer.front())) == length)
		{
			ShowImage(buffer);
		}
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
				PostQuitMessage(0);
				break;
			}
			case WM_SIZE:
			{
				g_mainWindow->OnResize();
				break;
			}
			case WM_PAINT:
			{
				g_mainWindow->OnPaint();
				break;
			}
			case WM_INITDIALOG:
			{
				InitCommonControls();
			}
			case WM_KEYUP:
			{
				g_mainWindow->OnKeyUp(wParam);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				g_mainWindow->OnDoubleClick();
				break;
			}
			case WM_COMMAND:
			{
				g_mainWindow->OnCommand(wParam);
				break;
			}
			case WM_DROPFILES:
			{
				g_mainWindow->OnFileDrop(wParam);
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