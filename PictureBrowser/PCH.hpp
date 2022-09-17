#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <gdiplus.h>

#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <map>

namespace PictureBrowser
{
	template <typename T>
	void Clear(T* x)
	{
		memset(x, 0, sizeof(T));
	}

	constexpr UINT Padding = 5;
}