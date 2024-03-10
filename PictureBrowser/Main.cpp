#include "PCH.hpp"
#include "MainWindow.hpp"
#include "Resource.h"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	class ComEnvironment
	{
	public:
		ComEnvironment() :
			_result(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))
		{
		}

		~ComEnvironment()
		{
			if (_result != S_FALSE)
			{
				CoUninitialize();
			}
		}

		operator bool() const
		{
			return _result == S_OK;
		}
	private:
		HRESULT _result = S_FALSE;
	};

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

	const ComEnvironment comEnv;

	if (!comEnv)
	{
		return ERROR_BAD_ENVIRONMENT;
	}

	MSG message;
	ZeroInit(&message);

	{
		MainWindow mainWindow(instance);

		try
		{
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
					DispatchMessageW(&message);
				}
			} while (run != 0);
		}
		catch (const std::system_error& e)
		{
			MessageBoxA(nullptr,
				e.what(),
				"An unhandled exception occurred!",
				MB_ICONSTOP | MB_OK);
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr,
				e.what(),
				"An unhandled exception occurred!",
				MB_ICONSTOP | MB_OK);
		}
	}

	return static_cast<int>(message.wParam);
}