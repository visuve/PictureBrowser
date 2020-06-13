#include "PCH.hpp"
#include "KeyboardHandler.hpp"

KeyboardHandler::KeyboardHandler(HWND window, HWND fileListBox, std::function<void(LONG_PTR)> selectImage) :
	m_window(window),
	m_fileListBox(fileListBox),
	m_selectImage(selectImage)
{
}

void KeyboardHandler::OnKeyUp(WPARAM wParam)
{
	const LONG_PTR count = SendMessage(m_fileListBox, LB_GETCOUNT, 0, 0);

	if (!count)
	{
		return;
	}

	switch (wParam)
	{
		case VK_LEFT:
		case VK_UP:
		{
			LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

			if (current > 0)
			{
				m_selectImage(--current);
			}

			return;
		}
		case VK_RIGHT:
		case VK_DOWN:
		{
			const LONG_PTR lastIndex = count - 1;
			LONG_PTR current = SendMessage(m_fileListBox, LB_GETCURSEL, 0, 0);

			if (current < lastIndex)
			{
				m_selectImage(++current);
			}

			return;
		}
	}
}
