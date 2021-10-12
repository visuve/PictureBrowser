#include "PCH.hpp"
#include "KeyboardHandler.hpp"

KeyboardHandler::KeyboardHandler(HWND window, HWND fileListBox, std::function<void(LONG_PTR)> selectImage) :
	_window(window),
	_fileListBox(fileListBox),
	_selectImage(selectImage)
{
}

void KeyboardHandler::OnKeyUp(WPARAM wParam)
{
	const LONG_PTR count = SendMessage(_fileListBox, LB_GETCOUNT, 0, 0);

	if (!count)
	{
		return;
	}

	switch (wParam)
	{
		case VK_LEFT:
		case VK_UP:
		{
			LONG_PTR current = SendMessage(_fileListBox, LB_GETCURSEL, 0, 0);

			if (current > 0)
			{
				_selectImage(--current);
			}

			return;
		}
		case VK_RIGHT:
		case VK_DOWN:
		{
			const LONG_PTR lastIndex = count - 1;
			LONG_PTR current = SendMessage(_fileListBox, LB_GETCURSEL, 0, 0);

			if (current < lastIndex)
			{
				_selectImage(++current);
			}

			return;
		}
	}
}
