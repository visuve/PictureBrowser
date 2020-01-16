#pragma once

class LogWrap
{
public:
	template<std::size_t N>
	LogWrap(const wchar_t(&function)[N], int line)
	{
		const auto now = std::chrono::system_clock::now();
		const auto since = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());		
		*this << since.count() << L' ' << function << L':' << line << L": ";
	}

	~LogWrap();

	// Because #include <sstream> grows the binary by ~200Kb...
	LogWrap& operator << (int8_t value);
	LogWrap& operator << (int16_t value);
	LogWrap& operator << (int32_t value);
	LogWrap& operator << (int64_t value);

	LogWrap& operator << (uint8_t value);
	LogWrap& operator << (uint16_t value);
	LogWrap& operator << (uint32_t value);
	LogWrap& operator << (uint64_t value);

	LogWrap& operator << (wchar_t value);
	
	template<std::size_t N> 
	LogWrap& operator << (const wchar_t(&value)[N]) { m_buffer.append(value); return *this; }

	LogWrap& operator << (const std::wstring& value);

private:
	std::wstring m_buffer;
};

#define LOGD LogWrap(__FUNCTIONW__, __LINE__)