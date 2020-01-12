#pragma once

namespace PictureBrowser
{
	class MainWindow
	{
	public:
		MainWindow();
		~MainWindow();

		bool InitInstance(HINSTANCE, int);

	private:
		bool LoadStrings();
		ATOM Register() const;

		void OnCreate(HWND);
		void OnResize(HWND);
		void OnPaint(HWND) const;
		void OnKeyUp(HWND, WPARAM);
		void OnCommand(HWND, WPARAM);
		void OnFileDrop(HWND, WPARAM);
		void OnOpen(HWND);

		void LoadFileList(const std::filesystem::path&);
		void ChangeImage(HWND, const std::filesystem::path&);

		static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
		static INT_PTR CALLBACK GenericOkDialog(HWND, UINT, WPARAM, LPARAM);

		std::wstring m_title;
		std::wstring m_windowClassName;
		HWND m_prevButton = nullptr;
		HWND m_nextButton = nullptr;
		HINSTANCE m_instance = nullptr;
		RECT m_canvasSize = { 0 };

		std::unique_ptr<Gdiplus::Image> m_image;
		std::vector<std::filesystem::path> m_files;
		std::vector<std::filesystem::path>::const_iterator m_iter;
	};
}