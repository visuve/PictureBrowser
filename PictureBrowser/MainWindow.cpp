#include "PCH.hpp"
#include "Resource.h"
#include "MainWindow.hpp"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"

namespace PictureBrowser
{
	MainWindow* g_mainWindow = nullptr;

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
		switch (LoadFileList(path))
		{
			case std::filesystem::file_type::regular:
			{
				LoadPicture(path);
				break;
			}
			case std::filesystem::file_type::directory:
			{
				SelectImage(0);
				break;
			}
			default:
			{
				if (m_image)
				{
					m_image.reset();
					InvalidateRect(m_window, &m_canvasSize, true);
				}
				break;
			}
		}
	}

	std::filesystem::file_type MainWindow::LoadFileList(const std::filesystem::path& path)
	{
		std::wstring filter = L"\\*.jpg";

		const std::filesystem::file_type status = std::filesystem::status(path).type();

		switch (status)
		{
			case std::filesystem::file_type::none:
			case std::filesystem::file_type::not_found:
			case std::filesystem::file_type::unknown:
			{
				const std::wstring message =
					L"The path you have entered does not appear to exist:\n" + path.wstring();

				MessageBox(m_window,
					message.c_str(),
					L"I/O error!",
					MB_OK | MB_ICONINFORMATION);

				return status;
			}
			case std::filesystem::file_type::regular:
			{
				if (_wcsicmp(path.extension().c_str(), L".jpg") != 0)
				{
					MessageBox(m_window,
						L"Only the picture format of the future is supported.",
						L"Unsupported file format!",
						MB_OK | MB_ICONINFORMATION);

					return std::filesystem::file_type::none;
				}

				filter.insert(0, path.parent_path().wstring());
				break;
			}
			case std::filesystem::file_type::directory:
			{
				filter.insert(0, path.wstring());
				break;
			}
			default:
			{
				const std::wstring message =
					L"The path you have entered does not appear to be a file or a folder:\n" + path.wstring();

				MessageBox(m_window,
					message.c_str(),
					L"FUBAR",
					MB_OK | MB_ICONINFORMATION);

				return status;
			}
		}

		if (!DlgDirList(m_window, &filter.front(), IDC_LISTBOX, 0, DDL_READWRITE))
		{
			LOGD << L"DlgDirList failed with filter: " << filter;
			return std::filesystem::file_type::none;
		}

		const LONG_PTR count = SendMessage(m_fileListBox, LB_GETCOUNT, 0, 0);

		if (!count)
		{
			const std::wstring message =
				L"The path you have entered does not appear to have JPG files!\n" + path.wstring();

			MessageBox(
				m_window,
				message.c_str(),
				L"Empty directory!",
				MB_OK | MB_ICONINFORMATION);

			return std::filesystem::file_type::none;
		}

		if (!SendMessage(m_fileListBox, LB_SELECTSTRING, 0, reinterpret_cast<LPARAM>(path.filename().c_str())))
		{
			LOGD << L"Failed to send message LB_SELECTSTRING!";
		}

		return status;
	}

	void MainWindow::LoadPicture(const std::filesystem::path& path)
	{
		if (!std::filesystem::is_regular_file(path))
		{
			const std::wstring message =
				path.wstring() + L" does not appear to be a file!";

			MessageBox(m_window,
				message.c_str(),
				L"Unsupported file format!",
				MB_OK | MB_ICONINFORMATION);

			return;
		}

		m_image.reset(Gdiplus::Image::FromFile(path.c_str()));
		m_zoomPercent = 0;

		if (!m_image)
		{
			const std::wstring message =
				L"Failed to load:\n" + path.wstring();

			MessageBox(m_window,
				message.c_str(),
				L"FUBAR",
				MB_OK | MB_ICONINFORMATION);

			return;
		}

		const Gdiplus::RotateFlipType rotation = GdiExtensions::GetRotation(m_image.get());

		if (rotation != Gdiplus::RotateFlipType::RotateNoneFlipNone)
		{
			m_image->RotateFlip(rotation);
		}

		InvalidateRect(m_window, &m_canvasSize, false);

		const std::wstring title = m_title + L" - " + path.filename().wstring();
		SetWindowText(m_window, title.c_str());
	}

	void MainWindow::OnCreate(HWND window)
	{
		m_zoomOutButton = CreateWindow(
			WC_BUTTON,
			L"-",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			300,
			0,
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
			450,
			0,
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
			300,
			710,
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
			WS_VISIBLE | WS_CHILD | LBS_STANDARD,
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

			if (!SetWindowPos(
				m_zoomOutButton,
				HWND_TOP,
				m_canvasSize.left,
				m_canvasSize.top,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER))
			{
				LOGD << L"Failed move minus button!";
			}

			if (!SetWindowPos(
				m_zoomInButton,
				HWND_TOP,
				m_canvasSize.right - ButtonWidth,
				m_canvasSize.top,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER))
			{
				LOGD << L"Failed move plus button!";
			}

			if (!SetWindowPos(
				m_previousPictureButton,
				HWND_TOP,
				m_canvasSize.left,
				m_canvasSize.bottom - ButtonHeight,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER))
			{
				LOGD << L"Failed move previous button!";
			}

			if (!SetWindowPos(
				m_nextPictureButton,
				HWND_TOP,
				m_canvasSize.right - ButtonWidth,
				m_canvasSize.bottom - ButtonHeight,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER))
			{
				LOGD << L"Failed to move next button!";
			}

			if (!SetWindowPos(
				m_fileListBox,
				HWND_TOP,
				0,
				0,
				FileListWidth,
				m_canvasSize.bottom, // TODO: there remains some weird gap
				SWP_NOMOVE | SWP_NOZORDER))
			{
				LOGD << L"Failed to move file list!";
			}
		}
	}

	void MainWindow::OnPaint() const
	{
		GdiExtensions::ContextWrapper context(m_window);

		if (!context.IsValid())
		{
			return;
		}

		Gdiplus::Graphics& graphics = context.Get();
		const Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color::DarkGray);
		graphics.FillRectangle(&mySolidBrush, m_canvasSize.left, m_canvasSize.top, m_canvasSize.right, m_canvasSize.bottom);

		if (!m_image)
		{
			return;
		}

		Gdiplus::Rect rect = GdiExtensions::ScaleToCanvasSize(m_canvasSize, m_image->GetWidth(), m_image->GetHeight());
		GdiExtensions::Zoom(rect, m_zoomPercent);
		graphics.DrawImage(m_image.get(), rect);
	}

	void MainWindow::OnDoubleClick()
	{
		if (ShowWindow(m_window, m_maximized ? SW_NORMAL : SW_SHOWMAXIMIZED))
		{
			m_maximized = !m_maximized;
		}
	}

	void MainWindow::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (m_zoomPercent > 0)
				{
					m_zoomPercent -= 5;
					InvalidateRect(m_window, &m_canvasSize, true);
				}
				break;
			case VK_OEM_PLUS:
				if (m_zoomPercent < 1000)
				{
					m_zoomPercent += 5;
					InvalidateRect(m_window, &m_canvasSize, false);
				}
				break;
		}

		LOGD << m_zoomPercent;
	}

	void MainWindow::OnKeyUp(WPARAM wParam)
	{
		const LONG_PTR count = SendMessage(m_fileListBox, LB_GETCOUNT, 0, 0);

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
			case VK_UP:
			{
				LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

				if (current > 0)
				{
					SelectImage(--current);
				}

				return;
			}
			case VK_RIGHT:
			case VK_DOWN:
			{
				const LONG_PTR lastIndex = count -1;
				LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

				if (current < lastIndex)
				{
					SelectImage(++current);
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
			case IDC_ZOOM_OUT_BUTTON:
			{
				OnZoom(VK_OEM_MINUS);
				break;
			}
			case IDC_ZOOM_IN_BUTTON:
			{
				OnZoom(VK_OEM_PLUS);
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

		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
			{
				OnSelectionChanged();
				break;
			}
		}
	}

	void MainWindow::OnFileDrop(WPARAM wParam)
	{
		const HDROP dropInfo = reinterpret_cast<HDROP>(wParam);
		const UINT required = DragQueryFile(dropInfo, 0, nullptr, 0);
		std::wstring path(required, L'\0');

		if (required)
		{
			path.resize(required);

			if (DragQueryFile(dropInfo, 0, &path.front(), required + 1))
			{
				Open(path);
			}
		}

		DragFinish(dropInfo);
		SetFocus(m_window); // Somehow loses focus without
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

		if (GetOpenFileName(&openFile))
		{
			Open(openFile.lpstrFile);
		}
		else
		{
			LOGD << L"Failed to get path!";
		}
	}

	void MainWindow::OnSelectionChanged()
	{
		const std::filesystem::path path = SelectedImage();

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
	}

	void MainWindow::OnDestroy()
	{
		g_mainWindow->m_image.reset();
	}

	std::filesystem::path MainWindow::ImageFromIndex(LONG_PTR index) const
	{
		const size_t length = static_cast<size_t>(SendMessage(m_fileListBox, LB_GETTEXTLEN, index, 0));
		std::wstring buffer(length, '\0');

		if (SendMessage(m_fileListBox, LB_GETTEXT, index, reinterpret_cast<LPARAM>(&buffer.front())) != length)
		{
			LOGD << L"Failed to send LB_GETTEXT!";
			return {};
		}

		return buffer;
	}

	std::filesystem::path MainWindow::SelectedImage() const
	{
		const LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

		if (current < 0)
		{
			LOGD << L"Failed to get current index or nothing selected. Got: " << current;
			return {};
		}

		return ImageFromIndex(current);
	}

	void MainWindow::SelectImage(LONG_PTR current)
	{
		if (SendMessage(m_fileListBox, LB_SETCURSEL, current, 0) < 0)
		{
			LOGD << L"Failed to send LB_SETCURSEL!";
			return;
		}

		const std::filesystem::path path = ImageFromIndex(current);

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
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
				g_mainWindow->OnDestroy();
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