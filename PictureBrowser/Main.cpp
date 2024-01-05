#include "PCH.hpp"
#include "MainWindow.hpp"
#include "Resource.h"
#include "LogWrap.hpp"
#include "GdiExtensions.hpp"

namespace PictureBrowser
{
	std::filesystem::path TrimQuotes(std::wstring_view path)
	{
		if (path.front() == '"' && path.back() == '"')
		{
			path.remove_prefix(1);
			path.remove_suffix(1);
			return path;
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

	MSG message;
	ZeroInit(&message);

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
			run = GetMessage(&message, nullptr, 0, 0);

			if (run == -1)
			{
				LOGD << uint32_t(GetLastError());
				break;
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		} while (run != 0);
	}

	return static_cast<int>(message.wParam);
}