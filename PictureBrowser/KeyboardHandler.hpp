#pragma once

class KeyboardHandler
{
public:
	KeyboardHandler(HWND window, HWND fileListBox, std::function<void(LONG_PTR)> selectImage);
	void OnKeyUp(WPARAM);

private:
	HWND m_window;
	HWND m_fileListBox;
	std::function<void(LONG_PTR)> m_selectImage;
};

