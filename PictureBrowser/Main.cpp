#include "PCH.hpp"
#include "MainWindow.hpp"
#include "Resource.h"

class GdiPlusGuard
{
public:
	GdiPlusGuard() :
		m_status(Gdiplus::GdiplusStartup(&m_gdiPlusToken, &gdiPlusStartupInput, nullptr))
	{
	}

	~GdiPlusGuard()
	{
		if (m_gdiPlusToken)
		{
			Gdiplus::GdiplusShutdown(m_gdiPlusToken);
		}
	}

	operator bool() const
	{
		return m_status == Gdiplus::Status::Ok;
	}
private:
	Gdiplus::GdiplusStartupInput gdiPlusStartupInput = { 0 };
	UINT_PTR m_gdiPlusToken = 0;
	Gdiplus::Status m_status;
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

int APIENTRY wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE previousInstance,
	_In_ LPWSTR commandLine,
	_In_ int showCommand)
{
	UNREFERENCED_PARAMETER(previousInstance);

	const GdiPlusGuard gdiGuard;
	PictureBrowser::MainWindow mainWindow;

	if (!gdiGuard || !mainWindow.InitInstance(instance, showCommand))
	{
		return GetLastError();
	}

	if (commandLine && commandLine[0] != '\0')
	{
		mainWindow.Display(TrimQuotes(commandLine));
	}

	MSG message = { 0 };

	while (GetMessage(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return static_cast<int>(message.wParam);
}