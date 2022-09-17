#pragma once

namespace PictureBrowser
{
	class KeyboardHandler
	{
	public:
		KeyboardHandler(HWND window, HWND fileListBox, std::function<void(LONG_PTR)> selectImage);
		void OnKeyUp(WPARAM);

	private:
		HWND _window;
		HWND _fileListBox;
		std::function<void(LONG_PTR)> _selectImage;
	};
}