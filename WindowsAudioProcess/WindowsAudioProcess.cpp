// WindowsAudioDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <string>

#include <atlbase.h>

#define INITGUID
#include <audioenginebaseapo.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>


#define GetName(T) L#T

using namespace ATL;

const wchar_t* AudioProcessingObjectsPath = L"AudioEngine\\AudioProcessingObjects\\";

const wchar_t* audioEndpointPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MMDevices\\Audio\\";
const wchar_t* fxPropertiesPath = L"\\FxProperties\\";



std::map<std::wstring, PROPERTYKEY> fxEfectMap =
{
	{GetName(PKEY_FX_StreamEffectClsid),PKEY_FX_StreamEffectClsid},
	{GetName(PKEY_FX_ModeEffectClsid),PKEY_FX_ModeEffectClsid},
	{GetName(PKEY_FX_EndpointEffectClsid),PKEY_FX_EndpointEffectClsid},
	{GetName(PKEY_FX_Offload_StreamEffectClsid),PKEY_FX_Offload_StreamEffectClsid},
	{GetName(PKEY_FX_Offload_ModeEffectClsid),PKEY_FX_Offload_ModeEffectClsid},
	{GetName(PKEY_CompositeFX_EndpointEffectClsid),PKEY_CompositeFX_EndpointEffectClsid}
};


std::vector<std::wstring> SpiltGuid(std::wstring guidString)
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
		else if(ch==L'}')
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

std::vector<std::wstring> GetActiveApos(const std::wstring endpointID, const  std::wstring& effectKey,std::wstring& endpointType)
{
	auto endpointKey = audioEndpointPath + std::wstring(L"\\") + endpointType + L"\\" + endpointID + fxPropertiesPath;
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


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
std::vector<APO_REG_PROPERTIES> GetAllApos()
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

std::wstring PropertyKeyToString(const PROPERTYKEY & key)
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
int main()
{
	std::wcout.imbue(std::locale("chs"));

	auto apoList = GetAllApos(); //Get all of APO

	CoInitializeEx(nullptr, 0);

	auto& clsid = __uuidof(MMDeviceEnumerator);
	auto& iid = __uuidof(IMMDeviceEnumerator);


	CComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT hr = CoCreateInstance(clsid, nullptr, CLSCTX_ALL, iid, (void**)&enumerator);

	CComPtr<IMMDevice> device;

	auto endpointType = EDataFlow::eCapture;
	hr = enumerator->GetDefaultAudioEndpoint(endpointType, ERole::eMultimedia, &device);

	

	CComPtr<IPropertyStore> props;
	hr = device->OpenPropertyStore(STGM_READ, &props);

	PROPVARIANT var;
	PropVariantInit(&var);
	hr = props->GetValue(PKEY_Device_FriendlyName, &var);
	auto friendlyName = var.pwszVal;
	std::wcout << L"Device_FriendlyName: " << friendlyName << std::endl;

	PropVariantInit(&var);
	hr = props->GetValue(PKEY_DeviceInterface_FriendlyName, &var);
	auto interfaceFriendlyName = var.pwszVal;
	std::wcout << L"DeviceInterface_FriendlyName: " << interfaceFriendlyName << std::endl;

	LPWSTR endpointID = nullptr;
	hr = device->GetId(&endpointID);
	std::wcout << "Endpoint ID: " << endpointID << std::endl;

	

	std::wstring endpointIDStr(endpointID);
	auto index = endpointIDStr.find_last_of(L'.');
	auto endpointIDGuid = endpointIDStr.substr(index+1, endpointIDStr.size() - index);


	std::for_each(fxEfectMap.begin(), fxEfectMap.end(), [&](std::pair<std::wstring,PROPERTYKEY> pair) 
	{
		auto clsidStr = PropertyKeyToString(pair.second);
		std::wstring endpointTypeStr;
		if (endpointType == EDataFlow::eRender)
		{
			endpointTypeStr = L"Render";
		}
		else
		{
			endpointTypeStr = L"Capture";
		}
		auto activeApoList = GetActiveApos(endpointIDGuid, clsidStr, endpointTypeStr);
		if (activeApoList.size() != 0)
		{
			std::wcout << pair.first << ":" << std::endl;
			std::for_each(activeApoList.begin(), activeApoList.end(), [&](std::wstring& item)
			{
				std::wstring name;
				std::wstring copyright;
				std::find_if(apoList.begin(), apoList.end(), [&item,&name,&copyright](APO_REG_PROPERTIES& prop)
				{
					CLSID tempClsid;
					CLSIDFromString(item.c_str(), &tempClsid);
					if (IsEqualCLSID(tempClsid, prop.clsid))
					{
						name = prop.szFriendlyName;
						copyright = prop.szCopyrightInfo;
						return true;
					}
					return false;
				});
				std::wcout << "    Name      :"<< name << std::endl;
				std::wcout << "    Copyright : " << copyright << std::endl;
				std::wcout << "    ClSID : " << item << std::endl;
			});
			std::wcout << std::endl << std::endl;
		}
	});
	
	
	
}

