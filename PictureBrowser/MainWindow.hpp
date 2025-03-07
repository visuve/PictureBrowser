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

		std::unique_ptr<Widget> _zoomOutButton;
		std::unique_ptr<Widget> _zoomInButton;
		std::unique_ptr<Widget> _previousPictureButton;
		std::unique_ptr<Widget> _nextPictureButton;

		RECT _fileListArea = { 0, 0, 0, 0 };
		RECT _mainArea = { 0, 0, 0, 0 };
		RECT _canvasArea = { 0, 0, 0, 0 };

		std::shared_ptr<ImageCache> _imageCache;
		std::unique_ptr<FileListWidget> _fileListWidget;
		std::unique_ptr<CanvasWidget> _canvasWidget;
	};
}