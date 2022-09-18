#include "PCH.hpp"
#include "FileListWidget.hpp"
#include "LogWrap.hpp"
#include "Resource.h"

namespace PictureBrowser
{
	class GlobalMemory
	{
	public:
		GlobalMemory(const void* data, size_t size, uint32_t flags = GMEM_MOVEABLE) :
			_memory(GlobalAlloc(flags, size))
		{
			if (_memory)
			{
				_lock = GlobalLock(_memory);
			}

			if (_lock)
			{
				memcpy(_lock, data, size);
			}
		}

		GlobalMemory(const GlobalMemory&) = delete;
		GlobalMemory(GlobalMemory&&) = delete;
		GlobalMemory& operator = (const GlobalMemory&) = delete;
		GlobalMemory& operator = (GlobalMemory&&) = delete;

		~GlobalMemory()
		{
			if (_lock)
			{
				GlobalUnlock(_memory);
			}
			else if (_memory)
			{
				// Did not acquire the lock, allocated for nothing
				GlobalFree(_memory);
			}
		}

		HGLOBAL Data() const
		{
			return _memory;
		}

	private:
		HGLOBAL _memory;
		void* _lock = nullptr;
	};

	FileListWidget::FileListWidget(
		HINSTANCE instance,
		HWND parent,
		const std::shared_ptr<ImageCache>& imageCache,
		const std::function<void(std::filesystem::path)>& imageChanged) :
		Widget(
			0,
			WC_LISTBOX,
			ClassName(FileListWidget),
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
			5,
			5,
			250,
			700,
			parent,
			reinterpret_cast<HMENU>(IDC_LISTBOX),
			instance,
			nullptr),
		_imageCache(imageCache),
		_imageChanged(imageChanged)
	{
	}

	void FileListWidget::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
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

				switch (LOWORD(wParam))
				{
					case IDC_PREV_BUTTON:
						MoveCurrentSelection(-1);
						break;
					case IDC_NEXT_BUTTON:
						MoveCurrentSelection(+1);
						break;
					case IDM_OPEN:
						OnOpenMenu();
						break;
					case IDM_POPUP_COPY_PATH:
						OnPopupClosed();
						break;
				}

				if (reinterpret_cast<HWND>(lParam) == _window && HIWORD(wParam) == LBN_SELCHANGE)
				{
					OnSelectionChanged();
					break;
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
				OnSelectionChanged();
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
		LONG_PTR current = CurrentSelection();
		return _currentDirectory / ImageFromIndex(current);
	}

	LONG_PTR FileListWidget::CurrentSelection() const
	{
		LONG_PTR count = Send(LB_GETCOUNT, 0, 0);
		LONG_PTR cursel = Send(LB_GETCURSEL, 0, 0);
		return std::clamp(cursel, LONG_PTR(0), count);
	}

	void FileListWidget::MoveCurrentSelection(LONG_PTR distance)
	{
		LONG_PTR count = Send(LB_GETCOUNT, 0, 0);
		LONG_PTR cursel = Send(LB_GETCURSEL, 0, 0);
		LONG_PTR nextsel = cursel + distance;

		if (cursel != nextsel && nextsel >= 0 && nextsel < count)
		{
			Send(LB_SETCURSEL, nextsel, 0);
			OnSelectionChanged(nextsel);
		}
	}

	void FileListWidget::OnSelectionChanged(LONG_PTR cursel)
	{
		if (cursel < 0)
		{
			cursel = CurrentSelection();
		}

		const std::filesystem::path path = _currentDirectory / ImageFromIndex(cursel);

		if (path.empty())
		{
			return;
		}

		LoadPicture(path);
	}

	void FileListWidget::OnOpenMenu()
	{
		OPENFILENAMEW openFile;
		ZeroInit(&openFile);

		std::wstring filePath(0x1000, '\0');

		openFile.lStructSize = sizeof(OPENFILENAMEW);
		openFile.hwndOwner = _parent;
		openFile.lpstrFile = filePath.data();
		openFile.nMaxFile = static_cast<DWORD>(filePath.size());
		openFile.lpstrFilter =
			L"Joint Photographic Experts Group (*.jpg)\0*.jpg\0Portable Network Graphics (*.png)\0*.png\0";
		openFile.nFilterIndex = 1;
		openFile.lpstrFileTitle = nullptr;
		openFile.nMaxFileTitle = 0;
		openFile.lpstrInitialDir = nullptr;
		openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&openFile))
		{
			Open(filePath);
		}
		else
		{
			LOGD << L"Failed to get path!";
		}
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
			std::unreachable();
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

		GlobalMemory memory(filename.c_str(), bytes);

		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, memory.Data());
		CloseClipboard();
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

		if (Send(LB_RESETCONTENT, 0, 0) != 0) // This message does not return a value.
		{
			std::unreachable();
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
		else if (Send(LB_SETCURSEL, 0, 0) == LB_ERR)
		{
			std::unreachable();
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

		if (_imageChanged)
		{
			_imageChanged(path);
		}
	}

	std::filesystem::path FileListWidget::ImageFromIndex(LONG_PTR index) const
	{
		LRESULT result = Send(LB_GETTEXTLEN, index, 0);

		if (result <= 0)
		{
			return {};
		}

		std::wstring buffer(static_cast<size_t>(result), '\0');

		if (Send(LB_GETTEXT, index, reinterpret_cast<LPARAM>(buffer.data())) != result)
		{
			std::unreachable();
		}

		return buffer;
	}
}