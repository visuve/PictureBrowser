#include "PCH.hpp"
#include "LogWrap.hpp"

LogWrap::~LogWrap()
{
	m_buffer.push_back('\n');
	OutputDebugString(m_buffer.c_str());
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

LogWrap& LogWrap::operator << (const std::wstring& value)
{
	m_buffer.append(value);
	return *this;
}