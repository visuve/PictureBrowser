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

	static const std::set<std::wstring> SupportedImageFormats = { L".jpg", L".png", L".bmp" };

	bool IsSupportedImageFormat(const std::filesystem::path& path)
	{
		return std::any_of(SupportedImageFormats.cbegin(), SupportedImageFormats.cend(), [&](const std::wstring& supported)
		{
			return path.extension() == supported;
		});
	}

	bool IsSupportedImageFormat(const std::filesystem::directory_entry& entry)
	{
		return IsSupportedImageFormat(entry.path());
	}

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

	bool MainWindow::InitCommonControls()
	{
		INITCOMMONCONTROLSEX icex = { 0 };
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES;
		return InitCommonControlsEx(&icex);
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

		if (!InitCommonControls())
		{
			LOGD << L"InitCommonControlsEx failed: " << uint32_t(GetLastError());
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
		m_imageCache.Clear();
		m_currentImage = nullptr;

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
				Invalidate();
				break;
			}
		}
	}

	std::filesystem::file_type MainWindow::LoadFileList(const std::filesystem::path& path)
	{
		const std::filesystem::file_type status = std::filesystem::status(path).type();

		m_currentDirectory.clear();
		m_fileList.clear();

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
				if (!IsSupportedImageFormat(path))
				{
					MessageBox(m_window,
						L"Only JPG and PNG are supported!",
						L"Unsupported file format!",
						MB_OK | MB_ICONINFORMATION);

					return std::filesystem::file_type::none;
				}

				m_currentDirectory = path.parent_path();
				break;
			}
			case std::filesystem::file_type::directory:
			{
				m_currentDirectory = path;
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

		if (!ListView_DeleteAllItems(m_fileListView))
		{
			LOGD << L"ListView_DeleteAllItems failed.";
		}

		int index = 0;
		int selectedIndex = 0;

		for (auto& iter : std::filesystem::directory_iterator(m_currentDirectory, std::filesystem::directory_options::skip_permission_denied))
		{
			if (!IsSupportedImageFormat(iter))
			{
				continue;
			}

			const std::wstring filename = iter.path().filename();
			m_fileList.push_back(filename);

			LVITEM item = { 0 };
			item.mask = LVIF_TEXT;
			item.cchTextMax = static_cast<int>(filename.size()) + 1;
			item.pszText = const_cast<wchar_t*>(m_fileList[index].c_str());
			item.iItem = index;
			
			if (ListView_InsertItem(m_fileListView, &item) == -1)
			{
				LOGD << L"ListView_InsertItem failed.";
			}

			if (iter == path)
			{
				selectedIndex = index;
			}

			++index;
		}

		if (m_fileList.empty())
		{
			const std::wstring message =
				L"The path you have entered does not appear to have JPG or PNG files!\n" + path.wstring();

			MessageBox(
				m_window,
				message.c_str(),
				L"Empty directory!",
				MB_OK | MB_ICONINFORMATION);

			return std::filesystem::file_type::none;
		}

		ListView_SetItemState(m_fileListView, selectedIndex, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
		ListView_EnsureVisible(m_fileListView, selectedIndex, false);

		return status;
	}

	bool MainWindow::LoadPicture(const std::filesystem::path& path)
	{
		if (!std::filesystem::is_regular_file(path))
		{
			const std::wstring message =
				path.wstring() + L" does not appear to be a file!";

			MessageBox(m_window,
				message.c_str(),
				L"Unsupported file format!",
				MB_OK | MB_ICONINFORMATION);

			return false;
		}

		m_zoomPercent = 0;
		m_mouseDragStart = { 0, 0 };
		m_mouseDragOffset = { 0, 0 };
		m_currentImage = m_imageCache.Get(path);

		if (!m_currentImage)
		{
			const std::wstring message =
				L"Failed to load:\n" + path.wstring();

			MessageBox(m_window,
				message.c_str(),
				L"FUBAR",
				MB_OK | MB_ICONINFORMATION);

			return false;
		}

		const std::wstring title = m_title + L" - " + path.filename().wstring();
		SetWindowText(m_window, title.c_str());

		Invalidate();

		return true;
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
		m_fileListArea.Height = clientArea.bottom - ButtonHeight - Padding * 3;

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

		m_fileListView = CreateWindow(
			WC_LISTVIEW,
			L"Filelist...",
			WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_LIST | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
			m_fileListArea.X,
			m_fileListArea.Y,
			m_fileListArea.Width,
			m_fileListArea.Height,
			window,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			m_instance,
			nullptr);

		if (ListView_SetExtendedListViewStyle(m_fileListView, LVS_EX_CHECKBOXES | LVS_EX_UNDERLINECOLD) != 0)
		{
			LOGD << L"ListView_SetExtendedListViewStyle failed.";
		}

		// ListView_SetBkColor(m_fileListView, RGB(255, 0, 0));
		// ListView_SetTextBkColor(m_fileListView, RGB(0, 255, 0));
		// ListView_SetTextColor(m_fileListView, RGB(0, 0, 255));
		// ListView_SetOutlineColor(m_fileListView, RGB(255, 0, 0));
		// ListView_SetInsertMarkColor(m_fileListView, RGB(255, 0, 0));

		m_deleteButton = CreateWindow(
			WC_BUTTON,
			L"Delete selected",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			m_fileListArea.X,
			m_fileListArea.Height + Padding * 2,
			m_fileListArea.Width,
			ButtonHeight,
			window,
			reinterpret_cast<HMENU>(IDC_DELETE_SELECTED_BUTTON),
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
	}

	void MainWindow::OnResize()
	{
		RecalculatePaintArea(m_window);

		if (!SetWindowPos(
			m_fileListView,
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
			m_deleteButton,
			HWND_TOP,
			m_fileListArea.X,
			m_fileListArea.Height + Padding * 2,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER))
		{
			LOGD << L"Failed move delete button!";
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

	void MainWindow::OnPaint() const
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

		if (m_currentImage)
		{
			Gdiplus::SizeF size(Gdiplus::REAL(m_currentImage->GetWidth()), Gdiplus::REAL(m_currentImage->GetHeight()));
			Gdiplus::Rect scaled;

			GdiExtensions::ScaleAndCenterTo(m_canvasArea, size, scaled);
			GdiExtensions::Zoom(scaled, m_zoomPercent);
			scaled.Offset(m_mouseDragOffset);

			if (m_isDragging)
			{
				const Gdiplus::Pen pen(Gdiplus::Color::Gray, 2.0f);
				buffer.DrawRectangle(&pen, scaled);
			}
			else
			{
				buffer.DrawImage(m_currentImage, scaled);
			}
		}

		context.Graphics().DrawImage(&bitmap, 0, 0, m_canvasArea.Width, m_canvasArea.Height);
	}

	void MainWindow::OnLeftMouseDown(LPARAM lParam)
	{
		if (!m_currentImage)
		{
			return;
		}

		const POINT point = { LOWORD(lParam), HIWORD(lParam) };
		m_isDragging = DragDetect(m_canvas, point);

		if (!m_isDragging)
		{
			return;
		}

		m_mouseDragStart.X = point.x - m_mouseDragOffset.X;
		m_mouseDragStart.Y = point.y - m_mouseDragOffset.Y;
	}

	void MainWindow::OnMouseMove(LPARAM lParam)
	{
		if (!m_isDragging)
		{
			return;
		}

		if (UpdateMousePositionOnCanvas(lParam))
		{
			Invalidate();
		}
	}

	void MainWindow::OnLeftMouseUp(LPARAM)
	{
		m_isDragging = false;
		Invalidate();
	}

	bool MainWindow::UpdateMousePositionOnCanvas(LPARAM lParam)
	{
		Gdiplus::Point distance(LOWORD(lParam) - m_mouseDragStart.X, HIWORD(lParam) - m_mouseDragStart.Y);

		if (distance.Equals(m_mouseDragOffset))
		{
			return false;
		}

		std::swap(m_mouseDragOffset, distance);
		return true;
	}

	void MainWindow::OnDoubleClick()
	{
		if (ShowWindow(m_window, m_maximized ? SW_NORMAL : SW_SHOWMAXIMIZED))
		{
			m_maximized = !m_maximized;
		}
	}

	void MainWindow::OnDeleteSelected()
	{
		MessageBox(m_window, L"Not implemented yet!", L"Picture Browser", MB_OK | MB_ICONINFORMATION);
	}

	void MainWindow::OnZoom(WPARAM wParam)
	{
		switch (wParam)
		{
			case VK_OEM_MINUS:
				if (m_zoomPercent > 0)
				{
					m_zoomPercent -= 5;
					m_mouseDragStart.X = int(m_mouseDragStart.X * 0.95f);
					m_mouseDragStart.Y = int(m_mouseDragStart.Y * 0.95f);
					m_mouseDragOffset.X = int(m_mouseDragOffset.X * 0.95f);
					m_mouseDragOffset.Y = int(m_mouseDragOffset.Y * 0.95f);
					Invalidate();
				}
				break;
			case VK_OEM_PLUS:
				if (m_zoomPercent < 1000)
				{
					m_zoomPercent += 5;
					m_mouseDragStart.X = int(m_mouseDragStart.X * 1.05f);
					m_mouseDragStart.Y = int(m_mouseDragStart.Y * 1.05f);
					m_mouseDragOffset.X = int(m_mouseDragOffset.X * 1.05f);
					m_mouseDragOffset.Y = int(m_mouseDragOffset.Y * 1.05f);
					Invalidate();
				}
				break;
		}

		LOGD << m_zoomPercent;
	}

	void MainWindow::OnKeyUp(WPARAM wParam)
	{
		const int count = ListView_GetItemCount(m_fileListView);
		const int lastIndex = count - 1;

		if (count <= 0)
		{
			return;
		}

		switch (wParam)
		{
			case VK_LEFT:
			case VK_UP:
			{
				int current = ListView_GetNextItem(m_fileListView, -1, LVNI_FOCUSED | LVNI_SELECTED);

				if (current > 0 && current <= lastIndex)
				{
					SelectImage(--current);
				}

				return;
			}
			case VK_RIGHT:
			case VK_DOWN:
			{
				int current = ListView_GetNextItem(m_fileListView, -1, LVNI_FOCUSED | LVNI_SELECTED);

				if (current >= 0 && current < lastIndex)
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
			case IDC_DELETE_SELECTED_BUTTON:
			{
				OnDeleteSelected();
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
		openFile.lpstrFilter = L"Joint Photographic Experts Group (*.jpg)\0*.jpg\0Portable Network Graphics (*.png)\0*.png\0";
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

	void MainWindow::OnSelectionChanged(size_t index)
	{
		const std::filesystem::path path = m_currentDirectory / m_fileList[index];

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
	}

	void MainWindow::OnDestroy()
	{
		m_currentDirectory.clear();
		m_currentImage = nullptr;
		m_imageCache.Clear();
	}

	void MainWindow::SelectImage(size_t index)
	{
		const std::filesystem::path path = m_currentDirectory / m_fileList[index];

		if (path.empty())
		{
			return;
		}

		if (LoadPicture(path))
		{
			ListView_SetItemState(m_fileListView, index, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
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
				g_mainWindow->OnDestroy();
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
			case WM_NOTIFY:
			{
				const auto notification = reinterpret_cast<NMLISTVIEW*>(lParam);

				if (notification && notification->hdr.code == LVN_ITEMCHANGED && notification->hdr.hwndFrom == g_mainWindow->m_fileListView && notification->uNewState & LVIS_SELECTED)
				{
					g_mainWindow->OnSelectionChanged(notification->iItem);
					break;
				}

				// NMCLICK https://docs.microsoft.com/fi-fi/windows/win32/controls/nm-click-list-view

				break;
			}
			case WM_KEYUP:
			{
				LOGD << L"PSAKA";
				g_mainWindow->OnKeyUp(wParam);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			{
				g_mainWindow->OnDoubleClick();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				g_mainWindow->OnLeftMouseDown(lParam);
				break;
			}
			case WM_MOUSEMOVE:
			{
				g_mainWindow->OnMouseMove(lParam);
				break;
			}
			case WM_LBUTTONUP:
			{
				g_mainWindow->OnLeftMouseUp(lParam);
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