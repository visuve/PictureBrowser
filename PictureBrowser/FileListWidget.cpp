#include "PCH.hpp"
#include "FileListWidget.hpp"
#include "LogWrap.hpp"
#include "Resource.h"

namespace PictureBrowser
{
	FileListWidget::FileListWidget(
		HWND parent,
		const std::shared_ptr<ImageCache>& imageCache,
		const std::function<void(std::filesystem::path)>& imageChanged) :
		Widget(
			0,
			WC_LISTBOX,
			L"Filelist...",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
			0,
			0,
			100,
			700,
			parent,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			GetModuleHandle(nullptr),
			nullptr),
		_imageCache(imageCache),
		_imageChanged(imageChanged)
	{
	}

	void FileListWidget::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
	{
		switch (message)
		{
			case WM_CONTEXTMENU:
				if (reinterpret_cast<HWND>(wParam) == _window)
				{
					OnContextMenu(lParam);
				}
				break;
			case WM_COMMAND:
				if (reinterpret_cast<HWND>(lParam) == _window && HIWORD(wParam) == LBN_SELCHANGE)
				{
					OnUpdateSelection();
					break;
				}

				if (LOWORD(wParam) == IDM_OPEN)
				{
					OnOpenMenu();
					break;
				}				
				
				if (LOWORD(wParam) == IDM_POPUP_COPY_PATH)
				{
					OnPopupClosed();
				}
				break;
			case WM_DROPFILES:
				OnFileDrop(wParam);
				break;
		}

	}

	void FileListWidget::Open(const std::filesystem::path& path)
	{
		_imageCache->Clear();

		switch (LoadFileList(path))
		{
			case std::filesystem::file_type::regular:
			{
				LoadPicture(path);
				break;
			}
			case std::filesystem::file_type::directory:
			{
				OnUpdateSelection();
				break;
			}
		}
	}

	void FileListWidget::Clear()
	{
		_currentDirectory.clear();
		_imageCache->Clear();
	}

	std::filesystem::path FileListWidget::SelectedImage() const
	{
		const LONG_PTR current = Send(LB_GETCURSEL, 0, 0);

		if (current < 0)
		{
			LOGD << L"Failed to get current index or nothing selected. Got: " << current;
			return {};
		}

		return _currentDirectory / ImageFromIndex(current);
	}

	void FileListWidget::OnOpenMenu()
	{
		OPENFILENAME openFile = { 0 };
		wchar_t filePath[0x1000] = { 0 };

		openFile.lStructSize = sizeof(openFile);
		openFile.hwndOwner = _parent;
		openFile.lpstrFile = filePath;
		openFile.nMaxFile = 0xFFF;
		openFile.lpstrFilter =
			L"Joint Photographic Experts Group (*.jpg)\0*.jpg\0Portable Network Graphics (*.png)\0*.png\0";
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

	void FileListWidget::OnSelectionChanged()
	{
		const std::filesystem::path path = SelectedImage();

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
	}

	void FileListWidget::OnFileDrop(WPARAM wParam)
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
		SetFocus(_window); // Somehow loses focus without
	}

	void FileListWidget::OnContextMenu(LPARAM lParam)
	{
		const int16_t x = LOWORD(lParam);
		const int16_t y = HIWORD(lParam);

		POINT p = { x, y };

		if (!ScreenToClient(_window, &p))
		{
			LOGD << L"ScreenToClient failed";
			return;
		}

		LRESULT result = Send(LB_ITEMFROMPOINT,	0, MAKELPARAM(p.x, p.y));

		if (result < 0)
		{
			LOGD << L"LB_ITEMFROMPOINT failed";
			return;
		}

		WORD index = LOWORD(result);

		if (HIWORD(result))
		{
			LOGD << L"The click is outside client area " << p;
			return;
		}

		_contextMenuIndex = index;

		HMENU menu = CreatePopupMenu();

		InsertMenu(menu, 0, MF_STRING, IDM_POPUP_COPY_PATH, L"Copy filename to clipboard");

		TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, _parent, nullptr);

		DestroyMenu(menu);
	}

	void FileListWidget::OnPopupClosed()
	{
		std::wstring filename = ImageFromIndex(_contextMenuIndex);

		if (filename.empty())
		{
			return;
		}

		size_t bytes = filename.size() * sizeof(wchar_t) + sizeof(wchar_t);

		HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);

		if (!memory)
		{
			return;
		}

		void* lock = GlobalLock(memory);

		if (!lock)
		{
			return;
		}

		memcpy(lock, filename.c_str(), bytes);

		GlobalUnlock(memory);

		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, memory);
		CloseClipboard();
	}

	void FileListWidget::OnUpdateSelection()
	{
		const LONG_PTR count = Send(LB_GETCOUNT, 0, 0);

		if (!count)
		{
			return;
		}				
		
		LONG_PTR current = std::clamp(Send(LB_GETCURSEL, 0, 0), LONG_PTR(0), count);

		const std::filesystem::path path = _currentDirectory / ImageFromIndex(current);

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
	}

	std::filesystem::file_type FileListWidget::LoadFileList(const std::filesystem::path& path)
	{
		std::wstring jpgFilter = L"\\*.jpg";
		std::wstring pngFilter = L"\\*.png";

		const std::filesystem::file_type status = std::filesystem::status(path).type();

		switch (status)
		{
			case std::filesystem::file_type::none:
			case std::filesystem::file_type::not_found:
			case std::filesystem::file_type::unknown:
			{
				const std::wstring message =
					L"The path you have entered does not appear to exist:\n" + path.wstring();

				MessageBox(_window,
					message.c_str(),
					L"I/O error!",
					MB_OK | MB_ICONINFORMATION);

				return status;
			}
			case std::filesystem::file_type::regular:
			{
				if (_wcsicmp(path.extension().c_str(), L".jpg") != 0 &&
					_wcsicmp(path.extension().c_str(), L".jpeg") != 0 &&
					_wcsicmp(path.extension().c_str(), L".png") != 0)
				{
					MessageBox(_window,
						L"Only JPG and PNG are supported!",
						L"Unsupported file format!",
						MB_OK | MB_ICONINFORMATION);

					return std::filesystem::file_type::none;
				}

				jpgFilter.insert(0, path.parent_path().wstring());
				pngFilter.insert(0, path.parent_path().wstring());
				_currentDirectory = path.parent_path();
				break;
			}
			case std::filesystem::file_type::directory:
			{
				jpgFilter.insert(0, path.wstring());
				pngFilter.insert(0, path.wstring());
				_currentDirectory = path;
				break;
			}
			default:
			{
				const std::wstring message =
					L"The path you have entered does not appear to be a file or a folder:\n" + path.wstring();

				MessageBox(_window,
					message.c_str(),
					L"FUBAR",
					MB_OK | MB_ICONINFORMATION);

				return status;
			}
		}

		if (!Send(LB_RESETCONTENT, 0, 0))
		{
			LOGD << L"Failed to send message LB_RESETCONTENT!";
		}

		if (!Send(LB_DIR, DDL_READWRITE, reinterpret_cast<LPARAM>(jpgFilter.c_str())))
		{
			LOGD << L"Failed to send JPG filter update!";
		}

		if (!Send(LB_DIR, DDL_READWRITE, reinterpret_cast<LPARAM>(pngFilter.c_str())))
		{
			LOGD << L"Failed to send PNG filter update!";
		}

		const LONG_PTR count = Send(LB_GETCOUNT, 0, 0);

		if (!count)
		{
			const std::wstring message =
				L"The path you have entered does not appear to have JPG or PNG files!\n" + path.wstring();

			MessageBox(
				_window,
				message.c_str(),
				L"Empty directory!",
				MB_OK | MB_ICONINFORMATION);

			return std::filesystem::file_type::none;
		}

		if (status == std::filesystem::file_type::regular &&
			!Send(LB_SELECTSTRING, 0, reinterpret_cast<LPARAM>(path.filename().c_str())))
		{
			LOGD << L"Failed to send message LB_SELECTSTRING!";
		}
		else if (!Send(LB_SETCURSEL, 0, 0))
		{
			LOGD << L"Failed to send message LB_SETCURSEL !";
		}

		return status;
	}

	void FileListWidget::LoadPicture(const std::filesystem::path& path)
	{
		if (!std::filesystem::is_regular_file(path))
		{
			const std::wstring message =
				path.wstring() + L" does not appear to be a file!";

			MessageBox(_parent,
				message.c_str(),
				L"Unsupported file format!",
				MB_OK | MB_ICONINFORMATION);

			return;
		}

		if (!_imageCache->SetCurrent(path))
		{
			const std::wstring message =
				L"Failed to load:\n" + path.wstring();

			MessageBox(_parent,
				message.c_str(),
				L"FUBAR",
				MB_OK | MB_ICONINFORMATION);

			return;
		}

		_imageChanged(path);
	}

	std::filesystem::path FileListWidget::ImageFromIndex(LONG_PTR index) const
	{
		LRESULT result = Send(LB_GETTEXTLEN, index, 0);

		if (result <= 0)
		{
			return {};
		}

		const size_t length = static_cast<size_t>(result);
		std::wstring buffer(length, '\0');

		if (Send(LB_GETTEXT, index, reinterpret_cast<LPARAM>(buffer.data())) != length)
		{
			LOGD << L"Failed to send LB_GETTEXT!";
			return {};
		}

		return buffer;
	}
}