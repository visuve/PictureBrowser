#include "PCH.hpp"
#include "Registry.hpp"

namespace Registry
{
	RegPath::RegPath(const std::wstring& path) :
		m_subKey(path.substr(0, path.rfind('\\'))),
		m_valueName(path.substr(path.rfind('\\') + 1, path.length()))
	{
	}

	RegPath::RegPath(const wchar_t* path) :
		RegPath(std::wstring(path))
	{
	}

	const wchar_t* RegPath::SubKeyName() const
	{
		return m_subKey.c_str();
	}

	const wchar_t* RegPath::ValueName() const
	{
		return m_valueName.c_str();
	}

	std::wstring RegPath::Padded() const
	{
		return L'"' + m_subKey + L'\\' + m_valueName + L'"';
	}

	RegHandle::RegHandle(const HKEY hive, const RegPath& path)
	{
		if (RegCreateKeyEx(
			hive,
			path.SubKeyName(),
			0,
			nullptr,
			0,
			KEY_READ | KEY_WRITE,
			nullptr,
			&m_key,
			nullptr) != ERROR_SUCCESS)
		{
			LOGD << L"RegCreateKeyEx failed.";
		}
	}

	RegHandle::~RegHandle()
	{
		if (m_key)
		{
			if (RegCloseKey(m_key) != ERROR_SUCCESS)
			{
				LOGD << L"RegCloseKey failed.";
			}
		}
	}

	bool RegHandle::IsValid()
	{
		return m_key != nullptr;
	}

	HKEY RegHandle::Get() const
	{
		return m_key;
	}

	PHKEY RegHandle::Ref()
	{
		return &m_key;
	}
}