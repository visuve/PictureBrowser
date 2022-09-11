#pragma once

#include "ImageCache.hpp"

class FileListHandler
{
public:
	FileListHandler(HWND window, HWND fileListBox, const std::shared_ptr<ImageCache>& imageCache, const std::function<void(std::filesystem::path)>& imageChanged);
	void Open(const std::filesystem::path& path);
	void Clear();
	std::filesystem::path SelectedImage() const;
	void OnOpenMenu();
	void OnSelectionChanged();
	void OnFileDrop(WPARAM wParam);
	void SelectImage(LONG_PTR current);
	void OnContextMenu(LPARAM);
	void OnPopupClosed();

private:
	std::filesystem::file_type LoadFileList(const std::filesystem::path&);
	void LoadPicture(const std::filesystem::path& path);
	std::filesystem::path ImageFromIndex(LONG_PTR index) const;

	HWND _window = nullptr;
	HWND _fileListBox = nullptr;
	std::shared_ptr<ImageCache> _imageCache;
	std::filesystem::path _currentDirectory;
	std::function<void(std::filesystem::path)> _imageChanged;
	WORD _contextMenuIndex = 0;
};