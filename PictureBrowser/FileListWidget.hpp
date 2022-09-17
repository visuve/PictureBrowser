#pragma once

#include "ImageCache.hpp"
#include "Window.hpp"

namespace PictureBrowser
{
	class FileListWidget : public Widget
	{
	public:
		FileListWidget(
			HINSTANCE instance,
			HWND parent,
			const std::shared_ptr<ImageCache>& imageCache, 
			const std::function<void(std::filesystem::path)>& imageChanged);

		void HandleMessage(UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR) override;

		void Open(const std::filesystem::path& path);
		void Clear();
		std::filesystem::path SelectedImage() const;

	private:
		void OnOpenMenu();
		void OnSelectionChanged();
		void OnFileDrop(WPARAM);
		void OnUpdateSelection();
		void OnContextMenu(LPARAM);
		void OnPopupClosed();

		std::filesystem::file_type LoadFileList(const std::filesystem::path&);
		void LoadPicture(const std::filesystem::path& path);
		std::filesystem::path ImageFromIndex(LONG_PTR index) const;

		std::shared_ptr<ImageCache> _imageCache;
		std::filesystem::path _currentDirectory;
		std::function<void(std::filesystem::path)> _imageChanged;
		WORD _contextMenuIndex = 0;
	};
}