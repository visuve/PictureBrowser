#pragma once

#include "ImageCache.hpp"
#include "FileListWidget.hpp"
#include "MouseHandler.hpp"
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
		void OnErase() const;
		void OnPaint();

		void OnImageChanged(std::filesystem::path);
		void OnZoom(WPARAM);
		void OnCommand(WPARAM);
		void Invalidate(bool erase = false);

		UINT CheckedState(UINT menuEntry) const;
		void SetCheckedState(UINT menuEntry, UINT state) const;

		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		int _zoomPercent = 0;
		HWND _canvas = nullptr;
		HWND _zoomOutButton = nullptr;
		HWND _zoomInButton = nullptr;
		HWND _previousPictureButton = nullptr;
		HWND _nextPictureButton = nullptr;
		Gdiplus::Rect _fileListArea;
		Gdiplus::Rect _mainArea;
		Gdiplus::Rect _canvasArea;

		std::shared_ptr<ImageCache> _imageCache;
		std::unique_ptr<FileListWidget> _fileListWidget;
		std::unique_ptr<MouseHandler> _mouseHandler;
	};
}