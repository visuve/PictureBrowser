#pragma once

#include "ImageCache.hpp"
#include "FileListWidget.hpp"
#include "CanvasWidget.hpp"
#include "Window.hpp"

namespace PictureBrowser
{
	class MainWindow : public Window
	{
	public:
		MainWindow(HINSTANCE);
		~MainWindow();

		void Open(const std::filesystem::path&);
		bool HandleMessage(UINT, WPARAM, LPARAM) override;

	private:
		void RecalculatePaintArea();
		void OnCreate();
		void OnResize();
		void OnCommand(WPARAM);
		void OnDoubleClick();

		UINT CheckedState(UINT menuEntry) const;
		void SetCheckedState(UINT menuEntry, UINT state) const;

		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		Widget _zoomOutButton;
		Widget _zoomInButton;
		Widget _previousPictureButton;
		Widget _nextPictureButton;
		RECT _fileListArea = {};
		RECT _mainArea = {};
		RECT _canvasArea = {};

		std::shared_ptr<ImageCache> _imageCache;
		std::unique_ptr<FileListWidget> _fileListWidget;
		std::unique_ptr<CanvasWidget> _canvasWidget;
	};
}