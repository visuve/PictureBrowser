#include "PCH.hpp"
#include "MainWindow.hpp"
#include "Resource.h"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"

namespace PictureBrowser
{
	std::filesystem::path TrimQuotes(const std::wstring& path)
	{
		std::wstring copy = path;

		if (path.front() == '"' && path.back() == '"')
		{
			copy.pop_back();
			copy.erase(copy.cbegin());
			return copy;
		}

		return path;
	}
}

int APIENTRY wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE previousInstance,
	_In_ LPWSTR commandLine,
	_In_ int showCommand)
{
	UNREFERENCED_PARAMETER(previousInstance);

	using namespace PictureBrowser;

	const GdiExtensions::Environment environment;

	if (!environment)
	{
		return GetLastError();
	}

	MSG msg = { 0 };

	{
		MainWindow mainWindow(instance);
		mainWindow.Show(showCommand);

		if (commandLine && commandLine[0] != '\0')
		{
			mainWindow.Open(TrimQuotes(commandLine));
		}

		int run = 0;

		do
		{
			run = GetMessage(&msg, nullptr, 0, 0);

			if (run == -1)
			{
				LOGD << uint32_t(GetLastError());
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} while (run != 0);
	}

	return static_cast<int>(msg.wParam);
}