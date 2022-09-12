#pragma once

#include "ImageCache.hpp"
#include "FileListHandler.hpp"
#include "KeyboardHandler.hpp"
#include "MouseHandler.hpp"

namespace PictureBrowser
{
	class MainWindow
	{
	public:
		MainWindow();
		~MainWindow();

		bool InitInstance(HINSTANCE, int);
		void Open(const std::filesystem::path&);

	private:
		ATOM Register() const;

		void RecalculatePaintArea(HWND);
		void OnCreate(HWND);
		void OnResize();
		void OnContextMenu(WPARAM, LPARAM);
		void OnErase() const;
		void OnPaint();

		void OnImageChanged(std::filesystem::path);
		void OnZoom(WPARAM);
		void OnCommand(WPARAM);
		void Invalidate(bool erase = false);

		UINT CheckedState(UINT menuEntry) const;
		void SetCheckedState(UINT menuEntry, UINT state) const;

		static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		int _zoomPercent = 0;
		HWND _window = nullptr;
		HWND _canvas = nullptr;
		HWND _zoomOutButton = nullptr;
		HWND _zoomInButton = nullptr;
		HWND _previousPictureButton = nullptr;
		HWND _nextPictureButton = nullptr;
		HWND _fileListBox = nullptr;
		HINSTANCE _instance = nullptr;
		Gdiplus::Rect _fileListArea;
		Gdiplus::Rect _mainArea;
		Gdiplus::Rect _canvasArea;

		std::shared_ptr<ImageCache> _imageCache;
		std::unique_ptr<FileListHandler> _fileListHandler;
		std::unique_ptr<KeyboardHandler> _keyboardHandler;
		std::unique_ptr<MouseHandler> _mouseHandler;
	};
}