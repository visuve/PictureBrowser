#pragma once

#include "ImageCache.hpp"
#include "Widget.hpp"

namespace PictureBrowser
{
	class FileListWidget : public Widget
	{
	public:
		FileListWidget() = default;
		FileListWidget(
			HINSTANCE instance,
			BaseWindow* parent,
			const std::shared_ptr<ImageCache>& imageCache, 
			const std::function<void(std::filesystem::path)>& imageChanged);

		void HandleMessage(UINT, WPARAM, LPARAM) override;

		void Open(const std::filesystem::path& path);
		void Clear();
		std::filesystem::path SelectedImage() const;

	private:
		LONG_PTR CurrentSelection() const;
		void MoveCurrentSelection(LONG_PTR distance);
		void OnSelectionChanged(LONG_PTR cursel = -1);

		void OnOpenMenu();
		void OnFileDrop(WPARAM);
		void OnContextMenu(LPARAM);
		void OnOpenPath() const;
		void OnCopyPath() const;
		void OnDeletePath() const;

		std::filesystem::file_type LoadFileList(const std::filesystem::path&);
		void LoadPicture(const std::filesystem::path& path);
		std::filesystem::path ImageFromIndex(LONG_PTR index) const;

		std::shared_ptr<ImageCache> _imageCache;
		std::filesystem::path _currentDirectory;
		std::function<void(std::filesystem::path)> _imageChanged;
		WORD _contextMenuIndex = 0;
	};
}