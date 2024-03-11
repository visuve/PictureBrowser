#include "PCH.hpp"
#include "LogWrap.hpp"

namespace PictureBrowser
{
	LogWrap::~LogWrap()
	{
		_buffer.push_back('\n');
		OutputDebugStringW(_buffer.c_str());
	}

	LogWrap& LogWrap::operator << (bool value)
	{
		_buffer.append(value ? L"true" : L"false");
		return *this;
	}

	LogWrap& LogWrap::operator << (int8_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (int16_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (int32_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (int64_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (uint8_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (uint16_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (uint32_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (uint64_t value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (float value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (double value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (wchar_t value)
	{
		_buffer.push_back(value);
		return *this;
	}

	LogWrap& LogWrap::operator << (DWORD value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (LSTATUS value)
	{
		_buffer.append(std::to_wstring(value));
		return *this;
	}

	LogWrap& LogWrap::operator << (std::wstring_view value)
	{
		_buffer.append(value);
		return *this;
	}

	LogWrap& LogWrap::operator << (const std::wstring& value)
	{
		_buffer.append(value);
		return *this;
	}

	LogWrap& LogWrap::operator << (const std::filesystem::path& value)
	{
		_buffer.append(value.wstring());
		return *this;
	}

	LogWrap& LogWrap::operator << (const POINT& point)
	{
		_buffer.append(std::format(L"X={} Y={}", point.x, point.y));
		return *this;
	}

	LogWrap& LogWrap::operator << (const SIZE& size)
	{
		_buffer.append(std::format(L"W={} H={}", size.cx, size.cy));
		return *this;
	}

	LogWrap& LogWrap::operator << (const RECT& rect)
	{
		_buffer.append(std::format(L"L={} T={} R={} B={}", rect.left, rect.top, rect.right, rect.bottom));
		return *this;
	}
}