#include "PCH.hpp"
#include "LogWrap.hpp"

LogWrap::~LogWrap()
{
	m_buffer.push_back('\n');
	OutputDebugString(m_buffer.c_str());
}

LogWrap& LogWrap::operator << (bool value)
{
	m_buffer.append(value ? L"true" : L"false");
	return *this;
}

LogWrap& LogWrap::operator << (int8_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (int16_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (int32_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (int64_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (uint8_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (uint16_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (uint32_t value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (uint64_t value)
{
	m_buffer.append(std::to_wstring(value)); 
	return *this;
}

LogWrap& LogWrap::operator << (wchar_t value)
{
	m_buffer.push_back(value);
	return *this;
}

LogWrap& LogWrap::operator << (DWORD value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (LSTATUS value)
{
	m_buffer.append(std::to_wstring(value));
	return *this;
}

LogWrap& LogWrap::operator << (const std::wstring& value)
{
	m_buffer.append(value);
	return *this;
}

LogWrap& LogWrap::operator << (const std::filesystem::path& value)
{
	m_buffer.append(value.wstring());
	return *this;
}

LogWrap& LogWrap::operator << (const Gdiplus::Rect& rect)
{
	m_buffer.append(L"X=");
	m_buffer.append(std::to_wstring(rect.X));
	m_buffer.append(1, L' ');

	m_buffer.append(L"Y=");
	m_buffer.append(std::to_wstring(rect.Y));
	m_buffer.append(1, L' ');

	m_buffer.append(L"W=");
	m_buffer.append(std::to_wstring(rect.Width));
	m_buffer.append(1, L' ');

	m_buffer.append(L"H=");
	m_buffer.append(std::to_wstring(rect.Height));
	m_buffer.append(1, L' ');

	return *this;
}

LogWrap& LogWrap::operator << (const RECT& rect)
{
	m_buffer.append(L"L=");
	m_buffer.append(std::to_wstring(rect.left));
	m_buffer.append(1, L' ');

	m_buffer.append(L"T=");
	m_buffer.append(std::to_wstring(rect.top));
	m_buffer.append(1, L' ');

	m_buffer.append(L"R=");
	m_buffer.append(std::to_wstring(rect.right));
	m_buffer.append(1, L' ');

	m_buffer.append(L"B=");
	m_buffer.append(std::to_wstring(rect.bottom));
	m_buffer.append(1, L' ');

	return *this;
}