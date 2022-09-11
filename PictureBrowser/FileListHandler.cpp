#include "PCH.hpp"
#include "FileListHandler.hpp"
#include "LogWrap.hpp"
#include "Resource.h"

FileListHandler::FileListHandler(
	HWND window, 
	HWND fileListBox, 
	const std::shared_ptr<ImageCache>& imageCache,
	const std::function<void(std::filesystem::path)>& imageChanged) :
	_window(window),
	_fileListBox(fileListBox),
	_imageCache(imageCache),
	_imageChanged(imageChanged)
{
}

void FileListHandler::Open(const std::filesystem::path& path)
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
			SelectImage(0);
			break;
		}
		default:
		{
			// Invalidate();
			break;
		}
	}
}

void FileListHandler::Clear()
{
	_currentDirectory.clear();
	_imageCache->Clear();
}

std::filesystem::path FileListHandler::SelectedImage() const
{
	const LONG_PTR current = SendMessage(_fileListBox, LB_GETCURSEL, 0, 0);

	if (current < 0)
	{
		LOGD << L"Failed to get current index or nothing selected. Got: " << current;
		return {};
	}

	return _currentDirectory / ImageFromIndex(current);
}

void FileListHandler::OnOpenMenu()
{
	OPENFILENAME openFile = { 0 };
	wchar_t filePath[0x1000] = { 0 };

	openFile.lStructSize = sizeof(openFile);
	openFile.hwndOwner = _window;
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

void FileListHandler::OnSelectionChanged()
{
	const std::filesystem::path path = SelectedImage();

	if (path.empty())
	{
		return;
	}

	LoadPicture(path);
}

void FileListHandler::OnFileDrop(WPARAM wParam)
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

void FileListHandler::OnContextMenu(LPARAM lParam)
{
	const long x = LOWORD(lParam);
	const long y = HIWORD(lParam);

	POINT p = { x, y };

	if (!ScreenToClient(_fileListBox, &p))
	{
		return;
	}

	LRESULT result = SendMessage(
		_fileListBox, 
		LB_ITEMFROMPOINT, 
		0,
		MAKELPARAM(p.x, p.y));

	if (result < 0)
	{
		return;
	}

	WORD index = LOWORD(result);
	
	if (HIWORD(result))
	{
		return; // Outside the client area
	}

	_contextMenuIndex = index;

	HMENU menu = CreatePopupMenu();

	InsertMenu(menu, 0, MF_STRING, IDC_POPUP_COPY, L"Copy filename to clipboard");

	TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, _window, nullptr);

	DestroyMenu(menu);
}

void FileListHandler::OnPopupClosed()
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

void FileListHandler::SelectImage(LONG_PTR current)
{
	if (SendMessage(_fileListBox, LB_SETCURSEL, current, 0) < 0)
	{
		LOGD << L"Failed to send LB_SETCURSEL!";
		return;
	}

	const std::filesystem::path path = _currentDirectory / ImageFromIndex(current);

	if (path.empty())
	{
		return;
	}

	LoadPicture(path);
}

std::filesystem::file_type FileListHandler::LoadFileList(const std::filesystem::path& path)
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

	if (!SendMessage(_fileListBox, LB_RESETCONTENT, 0, 0))
	{
		LOGD << L"Failed to send message LB_RESETCONTENT!";
	}

	if (!SendMessage(_fileListBox, LB_DIR, DDL_READWRITE, reinterpret_cast<LPARAM>(jpgFilter.c_str())))
	{
		LOGD << L"Failed to send JPG filter update!";
	}

	if (!SendMessage(_fileListBox, LB_DIR, DDL_READWRITE, reinterpret_cast<LPARAM>(pngFilter.c_str())))
	{
		LOGD << L"Failed to send PNG filter update!";
	}

	const LONG_PTR count = SendMessage(_fileListBox, LB_GETCOUNT, 0, 0);

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
		!SendMessage(_fileListBox, LB_SELECTSTRING, 0, reinterpret_cast<LPARAM>(path.filename().c_str())))
	{
		LOGD << L"Failed to send message LB_SELECTSTRING!";
	}
	else if (!SendMessage(_fileListBox, LB_SETCURSEL, 0, 0))
	{
		LOGD << L"Failed to send message LB_SETCURSEL !";
	}

	return status;
}

void FileListHandler::LoadPicture(const std::filesystem::path& path)
{
	if (!std::filesystem::is_regular_file(path))
	{
		const std::wstring message =
			path.wstring() + L" does not appear to be a file!";

		MessageBox(_window,
			message.c_str(),
			L"Unsupported file format!",
			MB_OK | MB_ICONINFORMATION);

		return;
	}

	if (!_imageCache->SetCurrent(path))
	{
		const std::wstring message =
			L"Failed to load:\n" + path.wstring();

		MessageBox(_window,
			message.c_str(),
			L"FUBAR",
			MB_OK | MB_ICONINFORMATION);

		return;
	}

	_imageChanged(path);
}

std::filesystem::path FileListHandler::ImageFromIndex(LONG_PTR index) const
{
	LRESULT result = SendMessage(_fileListBox, LB_GETTEXTLEN, index, 0);

	if (result <= 0)
	{
		return {};
	}

	const size_t length = static_cast<size_t>(result);
	std::wstring buffer(length, '\0');

	if (SendMessage(_fileListBox, LB_GETTEXT, index, reinterpret_cast<LPARAM>(buffer.data())) != length)
	{
		LOGD << L"Failed to send LB_GETTEXT!";
		return {};
	}

	return buffer;
}