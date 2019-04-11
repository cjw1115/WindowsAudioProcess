#pragma once
#include <string>
#include <vector>

#define INITGUID

#include <audioenginebaseapo.h>
#include <Functiondiscoverykeys_devpkey.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define AudioProcessingObjectsPath	L"AudioEngine\\AudioProcessingObjects\\"

#define AudioEndpointPath			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MMDevices\\Audio\\"
#define FxPropertiesPath			L"\\FxProperties\\"

class APOUtility
{
	std::vector<std::wstring> SpiltGuid(std::wstring guidString);
	
public:
	std::wstring PropertyKeyToString(const PROPERTYKEY & key);
	std::vector<APO_REG_PROPERTIES> EnumAllAPO();
	std::vector<std::wstring> GetActiveAPO(const std::wstring endpointID, const  std::wstring& effectKey, std::wstring& endpointType);
};

