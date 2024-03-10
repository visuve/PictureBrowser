#include "PCH.hpp"
#include "Registry.hpp"

namespace PictureBrowser::Registry
{
	RegPath::RegPath(std::wstring_view path) :
		_subKey(path.substr(0, path.rfind('\\'))),
		_valueName(path.substr(path.rfind('\\') + 1, path.length()))
	{
	}

	const wchar_t* RegPath::SubKeyName() const
	{
		return _subKey.c_str();
	}

	const wchar_t* RegPath::ValueName() const
	{
		return _valueName.c_str();
	}

	std::wstring RegPath::Padded() const
	{
		return L'"' + _subKey + L'\\' + _valueName + L'"';
	}

	RegHandle::RegHandle(const HKEY hive, const RegPath& path)
	{
		LSTATUS status = RegCreateKeyExW(
			hive,
			path.SubKeyName(),
			0,
			nullptr,
			0,
			KEY_READ | KEY_WRITE,
			nullptr,
			&_key,
			nullptr);

		if (status != ERROR_SUCCESS)
		{
			throw std::system_error(status, std::system_category(), "RegCreateKeyExW");
		}
	}

	RegHandle::~RegHandle()
	{
		if (IsValid())
		{
			RegCloseKey(_key);
		}
	}

	bool RegHandle::IsValid()
	{
		return _key != nullptr && _key != HKEY(-1);
	}

	HKEY RegHandle::Get() const
	{
		return _key;
	}

	PHKEY RegHandle::Ref()
	{
		return &_key;
	}
}