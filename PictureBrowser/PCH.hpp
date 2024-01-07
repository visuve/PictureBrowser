#pragma once

#define NOMINMAX

#include <Windows.h>
#include <CommCtrl.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <map>

namespace PictureBrowser
{
	template <typename T>
	constexpr void ZeroInit(T* x)
	{
		memset(x, 0, sizeof(T));
	}

	constexpr UINT Padding = 5;

	using Microsoft::WRL::ComPtr;
}