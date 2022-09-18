#pragma once

#pragma warning(push)

#pragma warning(disable:4458)

#include <SDKDDKVer.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <gdiplus.h>

#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <map>

#pragma warning(pop)

namespace PictureBrowser
{
	template <typename T>
	void ZeroInit(T* x)
	{
		memset(x, 0, sizeof(T));
	}

	constexpr UINT Padding = 5;
}