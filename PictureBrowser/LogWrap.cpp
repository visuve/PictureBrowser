#include "PCH.hpp"
#include "LogWrap.hpp"

LogWrap::~LogWrap()
{
	_buffer.push_back('\n');
	OutputDebugString(_buffer.c_str());
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

LogWrap& LogWrap::operator << (const Gdiplus::Rect& rect)
{
	_buffer.append(L"X=");
	_buffer.append(std::to_wstring(rect.X));
	_buffer.append(1, L' ');

	_buffer.append(L"Y=");
	_buffer.append(std::to_wstring(rect.Y));
	_buffer.append(1, L' ');

	_buffer.append(L"W=");
	_buffer.append(std::to_wstring(rect.Width));
	_buffer.append(1, L' ');

	_buffer.append(L"H=");
	_buffer.append(std::to_wstring(rect.Height));
	_buffer.append(1, L' ');

	return *this;
}

LogWrap& LogWrap::operator << (const RECT& rect)
{
	_buffer.append(L"L=");
	_buffer.append(std::to_wstring(rect.left));
	_buffer.append(1, L' ');

	_buffer.append(L"T=");
	_buffer.append(std::to_wstring(rect.top));
	_buffer.append(1, L' ');

	_buffer.append(L"R=");
	_buffer.append(std::to_wstring(rect.right));
	_buffer.append(1, L' ');

	_buffer.append(L"B=");
	_buffer.append(std::to_wstring(rect.bottom));
	_buffer.append(1, L' ');

	return *this;
}