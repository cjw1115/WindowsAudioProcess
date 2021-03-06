#include "pch.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <string>

#include <atlbase.h>

#include <mmdeviceapi.h>
#include "APOUtility.h"


#define GetName(T) L#T

using namespace ATL;

std::map<std::wstring, PROPERTYKEY> fxEfectMap =
{
	{GetName(PKEY_FX_StreamEffectClsid),PKEY_FX_StreamEffectClsid},
	{GetName(PKEY_FX_ModeEffectClsid),PKEY_FX_ModeEffectClsid},
	{GetName(PKEY_FX_EndpointEffectClsid),PKEY_FX_EndpointEffectClsid},
	{GetName(PKEY_FX_Offload_StreamEffectClsid),PKEY_FX_Offload_StreamEffectClsid},
	{GetName(PKEY_FX_Offload_ModeEffectClsid),PKEY_FX_Offload_ModeEffectClsid},
	{GetName(PKEY_CompositeFX_EndpointEffectClsid),PKEY_CompositeFX_EndpointEffectClsid}
};

int main()
{
	std::wcout.imbue(std::locale("chs"));

	APOUtility apoUtility;
	auto apoList = apoUtility.EnumAllAPO(); //Get all of APO

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
		auto clsidStr = apoUtility.PropertyKeyToString(pair.second);
		std::wstring endpointTypeStr;
		if (endpointType == EDataFlow::eRender)
		{
			endpointTypeStr = L"Render";
		}
		else
		{
			endpointTypeStr = L"Capture";
		}
		auto activeApoList = apoUtility.GetActiveAPO(endpointIDGuid, clsidStr, endpointTypeStr);
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

