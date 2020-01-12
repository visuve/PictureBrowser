#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"

namespace PictureBrowser
{
	MainWindow* g_mainWindow = nullptr;

	constexpr UINT ButtonWidth = 50;
	constexpr UINT ButtonHeight = 25;

	struct FindCloseGuard
	{
		void operator()(HANDLE handle) { FindClose(handle); }
	};

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
		float canvasWidth = static_cast<float>(canvasSize.right);
		float canvasHeight = static_cast<float>(canvasSize.bottom);

		float aspectRatio = min(
			canvasWidth / imageWidth,
			canvasHeight / imageHeight);

		UINT x = UINT(canvasWidth - imageWidth * aspectRatio) / 2;
		UINT y = UINT(canvasHeight - imageHeight * aspectRatio) / 2;
		UINT w = UINT(imageWidth * aspectRatio);
		UINT h = UINT(imageHeight * aspectRatio);

		return Gdiplus::Rect(x, y, w, h);
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

		m_files.clear();

		const auto parentPath = std::filesystem::path(path).parent_path();

		WIN32_FIND_DATA findFileData = { 0 };
		const std::wstring searchString = parentPath.wstring() + L"\\*.jpg";

		std::unique_ptr<void, FindCloseGuard> finder(FindFirstFile(searchString.c_str(), &findFileData));

		if (finder.get() != INVALID_HANDLE_VALUE)
		{
			do
			{
				m_files.emplace_back(parentPath / findFileData.cFileName);
			} while (FindNextFile(finder.get(), &findFileData));

			m_iter = std::find(m_files.cbegin(), m_files.cend(), path);
		}

		return !m_files.empty();
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

	void MainWindow::OnCreate()
	{
		g_mainWindow->m_prevButton = CreateWindow(
			WC_BUTTON,
			L"<",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			300,
			710,
			ButtonWidth,
			ButtonHeight,
			m_window,
			reinterpret_cast<HMENU>(IDC_PREV_BUTTON),
			m_instance,
			nullptr);

		g_mainWindow->m_nextButton = CreateWindow(
			WC_BUTTON,
			L">",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			450,
			710,
			ButtonWidth,
			ButtonHeight,
			m_window,
			reinterpret_cast<HMENU>(IDC_NEXT_BUTTON),
			m_instance,
			nullptr);
	}

	void MainWindow::OnResize()
	{
		RECT canvasSize = { 0 };

		if (GetClientRect(m_window, &canvasSize))
		{
			std::swap(m_canvasSize, canvasSize);

			SetWindowPos(
				m_prevButton,
				HWND_TOP,
				m_canvasSize.left + ButtonWidth,
				m_canvasSize.bottom - ButtonHeight * 2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);

			SetWindowPos(
				m_nextButton,
				HWND_TOP,
				m_canvasSize.right - ButtonWidth * 2,
				m_canvasSize.bottom - ButtonHeight * 2,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER);
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
		constexpr auto isEmpty = [](HWND window, const auto& list) -> bool
		{
			if (list.empty())
			{
				MessageBox(window,
					L"Please drag & drop a file or open from the menu!",
					L"No image selected!",
					MB_OK | MB_ICONINFORMATION);
				return true;
			}

			return false;
		};

		switch (wParam)
		{
			case VK_LEFT:
			{
				if (isEmpty(m_window, m_files))
				{
					return;
				}

				if (m_iter > m_files.cbegin())
				{
					--m_iter;
					ShowImage(*m_iter);
				}

				return;
			}
			case VK_RIGHT:
			{
				if (isEmpty(m_window, m_files))
				{
					return;
				}

				if (m_iter < --m_files.cend())
				{
					++m_iter;
					ShowImage(*m_iter);
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

	LRESULT CALLBACK MainWindow::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_CREATE:
			{
				g_mainWindow->OnCreate();
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