#pragma once

#define NOMINMAX

#include <Windows.h>
#include <Shlobj.h>
#include <CommCtrl.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <map>
#include <stdexcept>
#include <span>

namespace PictureBrowser
{
	template <typename T>
	constexpr void ZeroInit(T& x)
	{
		uint8_t* begin = reinterpret_cast<uint8_t*>(&x);
		const uint8_t* const end = reinterpret_cast<uint8_t*>(&x) + sizeof(T);

		while (begin < end)
		{
			*begin = 0;
			++begin;
		}
	}

	constexpr UINT Padding = 5;

	using Microsoft::WRL::ComPtr;
}