#pragma once

namespace PictureBrowser
{
	class MainWindow
	{
	public:
		MainWindow();
		~MainWindow();

		bool InitInstance(HINSTANCE, int);
		bool LoadFileList(const std::filesystem::path&);
		void ShowImage(const std::filesystem::path&);

	private:
		bool LoadStrings();
		ATOM Register() const;

		void OnCreate(HWND);
		void OnResize();
		void OnPaint() const;
		void OnDoubleClick();
		void OnKeyUp(WPARAM);
		void OnCommand(WPARAM);
		void OnFileDrop(WPARAM);
		void OnOpenMenu();

		void ChangeSelection(LONG_PTR);

		static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		std::wstring m_title;
		std::wstring m_windowClassName;
		HWND m_window = nullptr;
		HWND m_prevButton = nullptr;
		HWND m_nextButton = nullptr;
		HWND m_fileListBox = nullptr;
		HINSTANCE m_instance = nullptr;
		RECT m_canvasSize = { 0 };
		bool m_maximized = false;

		std::unique_ptr<Gdiplus::Image> m_image;
	};
}