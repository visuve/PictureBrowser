#pragma once

namespace PictureBrowser
{
	class LogWrap
	{
	public:
		template<std::size_t N>
		LogWrap(const wchar_t(&function)[N], int line)
		{
			auto now = std::chrono::system_clock::now();
			*this << std::format(L"{} {}:{}: ", now, function, line);
		}

		~LogWrap();

		template <typename T>
		LogWrap& operator << (T value) = delete;

		LogWrap& operator << (bool value);

		LogWrap& operator << (int8_t value);
		LogWrap& operator << (int16_t value);
		LogWrap& operator << (int32_t value);
		LogWrap& operator << (int64_t value);

		LogWrap& operator << (uint8_t value);
		LogWrap& operator << (uint16_t value);
		LogWrap& operator << (uint32_t value);
		LogWrap& operator << (uint64_t value);

		LogWrap& operator << (wchar_t value);
		LogWrap& operator << (DWORD value);
		LogWrap& operator << (LSTATUS value);

		template<std::size_t N>
		LogWrap& operator << (const wchar_t(&value)[N])
		{
			_buffer.append(value);
			return *this;
		}

		LogWrap& operator << (const std::wstring& value);

		LogWrap& operator << (const std::filesystem::path& value);

		LogWrap& operator << (const Gdiplus::Rect& rect);

		LogWrap& operator << (const POINT& point);

		LogWrap& operator << (const RECT& rect);

		LogWrap& operator << (const SIZE& size);

	private:
		std::wstring _buffer;
	};

	class NullStream
	{
	public:
		template <typename T>
		constexpr NullStream& operator << (T)
		{
			return *this;
		}
	};
}

#ifdef _DEBUG
#define LOGD PictureBrowser::LogWrap(__FUNCTIONW__, __LINE__)
#else
#define LOGD NullStream()
#endif

