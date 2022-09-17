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
		if (RegCreateKeyEx(
			hive,
			path.SubKeyName(),
			0,
			nullptr,
			0,
			KEY_READ | KEY_WRITE,
			nullptr,
			&_key,
			nullptr) != ERROR_SUCCESS)
		{
			LOGD << L"RegCreateKeyEx failed.";
		}
	}

	RegHandle::~RegHandle()
	{
		if (_key)
		{
			if (RegCloseKey(_key) != ERROR_SUCCESS)
			{
				LOGD << L"RegCloseKey failed.";
			}
		}
	}

	bool RegHandle::IsValid()
	{
		return _key != nullptr;
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