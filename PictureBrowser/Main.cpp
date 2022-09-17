#include "PCH.hpp"
#include "MainWindow.hpp"
#include "Resource.h"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	class GdiPlusGuard
	{
	public:
		GdiPlusGuard() :
			_status(Gdiplus::GdiplusStartup(&_gdiPlusToken, &_gdiPlusStartupInput, nullptr))
		{
		}

		~GdiPlusGuard()
		{
			if (_gdiPlusToken)
			{
				Gdiplus::GdiplusShutdown(_gdiPlusToken);
			}
		}

		operator bool() const
		{
			return _status == Gdiplus::Status::Ok;
		}
	private:
		Gdiplus::GdiplusStartupInput _gdiPlusStartupInput;
		ULONG_PTR _gdiPlusToken = 0;
		Gdiplus::Status _status;
	};


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

	const GdiPlusGuard gdiGuard;

	if (!gdiGuard)
	{
		return GetLastError();
	}

	MainWindow mainWindow(instance);

	if (!mainWindow.Show(showCommand))
	{
		return GetLastError();
	}

	if (commandLine && commandLine[0] != '\0')
	{
		mainWindow.Open(TrimQuotes(commandLine));
	}

	int run = 0;
	MSG msg = { 0 };

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

	return static_cast<int>(msg.wParam);
}