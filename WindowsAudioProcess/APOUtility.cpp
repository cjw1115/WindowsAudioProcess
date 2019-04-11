#include "pch.h"
#include "APOUtility.h"

std::vector<std::wstring> APOUtility::SpiltGuid(std::wstring guidString)
{
	std::vector<std::wstring> spiltList;
	bool isRecored = false;
	std::wstring temp;
	for (int i = 0; i < guidString.size(); i++)
	{
		auto& ch = guidString.c_str()[i];
		if (ch == L'{')
		{
			isRecored = true;
			temp += ch;
		}
		else if (ch == L'}')
		{
			isRecored = false;
			temp += ch;
			spiltList.push_back(temp);
			temp.clear();
		}
		else
		{
			if (isRecored)
			{
				temp += ch;
			}
		}
	}
	return spiltList;
}

std::wstring APOUtility::PropertyKeyToString(const PROPERTYKEY & key)
{
	WCHAR clsidStr[PKEYSTR_MAX];
	HRESULT hr = PSStringFromPropertyKey(key, clsidStr, ARRAYSIZE(clsidStr));

	int index = 0;
	while (clsidStr[index])
	{
		if (clsidStr[index] == L' ')
		{
			clsidStr[index] = L',';
		}
		index++;
	}
	return std::wstring(clsidStr);
}

std::vector<std::wstring> APOUtility::GetActiveAPO(const std::wstring endpointID, const  std::wstring& effectKey, std::wstring& endpointType)
{
	auto endpointKey = std::wstring(AudioEndpointPath) + std::wstring(L"\\") + endpointType + L"\\" + endpointID + FxPropertiesPath;
	HKEY fxPropertiesKey;
	wchar_t valueBuffer[1024]{ 0 };
	DWORD size = sizeof(valueBuffer);
	auto result = RegGetValue(HKEY_LOCAL_MACHINE, endpointKey.c_str(), effectKey.c_str(), RRF_RT_REG_SZ, NULL, valueBuffer, &size);

	if (result == ERROR_SUCCESS && size > sizeof(L""))
	{
		return SpiltGuid(std::wstring(valueBuffer, size));
	}
	return std::vector<std::wstring>(0);
}

std::vector<APO_REG_PROPERTIES> APOUtility::EnumAllAPO()
{
	std::vector<APO_REG_PROPERTIES> apos;
	HKEY aposKey;
	auto result = RegOpenKey(HKEY_CLASSES_ROOT, AudioProcessingObjectsPath, &aposKey);
	if (result == ERROR_SUCCESS)
	{
		WCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
		DWORD    cbName;                   // size of name string 

		WCHAR    achClass[MAX_PATH] = { 0 };  // buffer for class name 
		DWORD    cchClassName = MAX_PATH;  // size of class string 
		DWORD    cSubKeys = 0;               // number of subkeys 
		DWORD    cbMaxSubKey;              // longest subkey size 
		DWORD    cchMaxClass;              // longest class string 
		DWORD    cValues;              // number of values for key 
		DWORD    cchMaxValue;          // longest value name 
		DWORD    cbMaxValueData;       // longest value data 
		DWORD    cbSecurityDescriptor; // size of security descriptor 
		FILETIME ftLastWriteTime;      // last write time 

		DWORD i, retCode;

		WCHAR  achValue[MAX_VALUE_NAME];
		DWORD cchValue = MAX_VALUE_NAME;

		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			aposKey,                    // key handle 
			achClass,                // buffer for class name 
			&cchClassName,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			&cbMaxSubKey,            // longest subkey size 
			&cchMaxClass,            // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			&cbSecurityDescriptor,   // security descriptor 
			&ftLastWriteTime);       // last write time

		// Enumerate the subkeys, until RegEnumKeyEx() fails
		if (cSubKeys)
		{
			for (i = 0; i < cSubKeys; i++)
			{
				cbName = MAX_KEY_LENGTH;
				retCode = RegEnumKeyEx(aposKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
				if (retCode == ERROR_SUCCESS)
				{
					APO_REG_PROPERTIES prop;
					CLSIDFromString(achKey, &prop.clsid);
					DWORD size = sizeof(prop.szFriendlyName);
					RegGetValue(aposKey, achKey, L"FriendlyName", RRF_RT_REG_SZ, NULL, prop.szFriendlyName, &size);
					size = sizeof(prop.szCopyrightInfo);
					RegGetValue(aposKey, achKey, L"Copyright", RRF_RT_REG_SZ, NULL, prop.szCopyrightInfo, &size);
					WCHAR valueBuffer[1024];
					size = sizeof(valueBuffer);
					RegGetValue(aposKey, achKey, L"APOInterface0", RRF_RT_REG_SZ, NULL, valueBuffer, &size);
					IIDFromString(valueBuffer, &prop.iidAPOInterfaceList[0]);

					apos.push_back(prop);
				}
			}
		}
	}
	return apos;
}