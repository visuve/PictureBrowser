#pragma once

#include "LogWrap.hpp"

namespace PictureBrowser::Registry
{
	class RegPath
	{
	public:
		RegPath(std::wstring_view path);
		
		template<std::size_t N>
		RegPath(const wchar_t(&path)[N]) :
			RegPath(std::wstring_view(path, N))
		{
		}

		const wchar_t* SubKeyName() const;
		const wchar_t* ValueName() const;
		std::wstring Padded() const;

	private:
		const std::wstring _subKey;
		const std::wstring _valueName;
	};

	class RegHandle
	{
	public:
		RegHandle() = default;
		RegHandle(const HKEY hive, const RegPath& path);
		~RegHandle();
		bool IsValid();
		HKEY Get() const;
		PHKEY Ref();
	private:
		HKEY _key = nullptr;
	};

	template<typename T>
	T Get(const RegPath& path, const T defaultValue, HKEY hive = HKEY_CURRENT_USER) = delete;

	template<>
	inline uint32_t Get(const RegPath& path, const uint32_t defaultValue, HKEY hive)
	{
		DWORD actualType = 0;
		DWORD value = 0;
		DWORD dataSize = sizeof(DWORD);

		LSTATUS status = RegGetValueW(
			hive,
			path.SubKeyName(),
			path.ValueName(),
			RRF_RT_DWORD,
			&actualType,
			&value,
			&dataSize
		);

		if (status == ERROR_FILE_NOT_FOUND)
		{
			LOGD << path.Padded() << L" not found!";
			return defaultValue;
		}

		if (status != ERROR_SUCCESS)
		{
			LOGD << L"Reading from: " << path.Padded() << L" failed with LSTATUS: " << status;
			return defaultValue;
		}

		_ASSERTE(actualType == REG_DWORD);
		_ASSERTE(dataSize == sizeof(DWORD));
		LOGD << path.Padded() << L"=" << static_cast<uint32_t>(value);
		return static_cast<uint32_t>(value);
	}

	template<>
	inline bool Get(const RegPath& path, const bool defaultValue, HKEY hive)
	{
		return static_cast<bool>(Get<uint32_t>(path, defaultValue, hive));
	}

	template<typename T>
	bool Set(const RegPath& path, const T value, HKEY hive = HKEY_CURRENT_USER) = delete;

	template<>
	inline bool Set(const RegPath& path, const uint32_t value, HKEY hive)
	{
		RegHandle key(hive, path);

		union U32U8
		{
			uint32_t U32 = 0;
			uint8_t U8[4];
		};

		U32U8 u32u8;
		u32u8.U32 = value;

		DWORD dataSize = sizeof(DWORD);

		LSTATUS status = RegSetValueExW(
			key.Get(),
			path.ValueName(),
			0,
			REG_DWORD,
			u32u8.U8,
			dataSize
		);

		if (status != ERROR_SUCCESS)
		{
			LOGD << L"Setting: " << path.Padded() << L" failed with LSTATUS: " << status;
			return false;
		}

		LOGD << path.Padded() << L"=" << value;
		return true;
	}

	template<>
	inline bool Set(const RegPath& path, const bool value, HKEY hive)
	{
		return Set<uint32_t>(path, value, hive);
	}
}