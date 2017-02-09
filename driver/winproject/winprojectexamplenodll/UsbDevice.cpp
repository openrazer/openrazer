#include <cstdio>
#include <sstream>
#include <wchar.h>
#include <string.h>
#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "UsbDevice.h"

//Helper functions for native Windows USB
//These functions courtesy of http://www.reddit.com/user/chrisgzy
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

bool IsMatchingDevice(wchar_t *pDeviceID, unsigned int uiVID, unsigned int uiPID, unsigned int uiMI) {
    unsigned int uiLocalVID = 0, uiLocalPID = 0, uiLocalMI = 0;
    LPWSTR pszNextToken = 0;
    LPWSTR pszToken = wcstok(pDeviceID, L"\\#&");
    while (pszToken) {
        std::wstring tokenStr(pszToken);
        if (tokenStr.find(L"VID_", 0, 4) != std::wstring::npos) {
            std::wistringstream iss(tokenStr.substr(4));
            iss >> std::hex >> uiLocalVID;
        } else if (tokenStr.find(L"PID_", 0, 4) != std::wstring::npos) {
            std::wistringstream iss(tokenStr.substr(4));
            iss >> std::hex >> uiLocalPID;
        } else if (tokenStr.find(L"MI_", 0, 3) != std::wstring::npos) {
            std::wistringstream iss(tokenStr.substr(3));
            iss >> std::hex >> uiLocalMI;
        }
        pszToken = wcstok(0, L"\\#&");
    }
    if (uiVID != uiLocalVID || uiPID != uiLocalPID || uiMI != uiLocalMI)
        return false;
    return true;
}

HANDLE GetDeviceHandle(unsigned int uiVID, unsigned int uiPID, unsigned int uiMI) {
    const GUID GUID_DEVINTERFACE_HID = { 0x4D1E55B2L, 0xF16F, 0x11CF, 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 };
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_HID, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return 0;

    HANDLE hReturn = 0;

    SP_DEVINFO_DATA deviceData = { 0 };
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (unsigned int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceData); ++i) {
        wchar_t wszDeviceID[MAX_DEVICE_ID_LEN];
        if (CM_Get_Device_IDW(deviceData.DevInst, wszDeviceID, MAX_DEVICE_ID_LEN, 0))
            continue;

        if (!IsMatchingDevice(wszDeviceID, uiVID, uiPID, uiMI))
            continue;

        SP_INTERFACE_DEVICE_DATA interfaceData = { 0 };
        interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        if (!SetupDiEnumDeviceInterfaces(hDevInfo, &deviceData, &GUID_DEVINTERFACE_HID, 0, &interfaceData))
            break;

        DWORD dwRequiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, 0, 0, &dwRequiredSize, 0);

        SP_INTERFACE_DEVICE_DETAIL_DATA *pData = (SP_INTERFACE_DEVICE_DETAIL_DATA *)new unsigned char[dwRequiredSize];
        pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, pData, dwRequiredSize, 0, 0))
        {
            delete pData;
            break;
        }

        HANDLE hDevice = CreateFile(pData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
        if (hDevice == INVALID_HANDLE_VALUE)
        {
            delete pData;
            break;
        }

        hReturn = hDevice;
        break;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return hReturn;
}

HANDLE UsbDevice::OpenDevice(unsigned short vendor, unsigned short product, unsigned int MI) {
    return GetDeviceHandle(vendor, product, MI);
}

bool UsbDevice::SendToDevice(HANDLE handle, unsigned char* data, unsigned int length) {
    return(HidD_SetFeature(handle, data, length));
}

HANDLE call_UsbDevice_OpenDevice(unsigned short vendor, unsigned short product, unsigned int MI) {
    return UsbDevice::OpenDevice(vendor, product, MI);
}

int call_UsbDevice_SendToDevice(HANDLE handle, unsigned char* data, unsigned int length) {
    return UsbDevice::SendToDevice(handle, data, length);
}

