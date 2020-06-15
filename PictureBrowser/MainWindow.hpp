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
		bool LoadStrings();
		ATOM Register() const;

		void RecalculatePaintArea(HWND);
		void OnCreate(HWND);
		void OnResize();
		void OnErase() const;
		void OnPaint();

		void OnImageChanged(std::filesystem::path);
		void OnZoom(WPARAM);
		void OnCommand(WPARAM);
		void Invalidate(bool erase = false);

		static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		std::wstring m_title;
		std::wstring m_windowClassName;
		int m_zoomPercent = 0;
		HWND m_window = nullptr;
		HWND m_canvas = nullptr;
		HWND m_zoomOutButton = nullptr;
		HWND m_zoomInButton = nullptr;
		HWND m_previousPictureButton = nullptr;
		HWND m_nextPictureButton = nullptr;
		HWND m_fileListBox = nullptr;
		HINSTANCE m_instance = nullptr;
		Gdiplus::Rect m_fileListArea;
		Gdiplus::Rect m_mainArea;
		Gdiplus::Rect m_canvasArea;

		std::shared_ptr<ImageCache> m_imageCache;
		std::unique_ptr<FileListHandler> m_fileListHandler;
		std::unique_ptr<KeyboardHandler> m_keyboardHandler;
		std::unique_ptr<MouseHandler> m_mouseHandler;
	};
}