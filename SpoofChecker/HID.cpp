#include "pch.h"

void HID::PrintHidString(HANDLE DeviceHandle, DWORD IoctlCode, const std::string& Label)
{
    WCHAR StringBuffer[256] = { 0 };
    DWORD BytesReturned = 0;

    if (DeviceIoControl(DeviceHandle, IoctlCode, nullptr, 0, StringBuffer, sizeof(StringBuffer), &BytesReturned, nullptr))
    {
        StringBuffer[ARRAYSIZE(StringBuffer) - 1] = L'\0';

        std::string AnsiString = UTILS::WStringToString(StringBuffer);
        if (!AnsiString.empty())
        {
            std::cout << "  " << "[" << dye::light_purple(Label) << "]" ": " << dye::green(AnsiString) << std::endl;
        }
    }
}

void HID::PrintCollectionInfo(HANDLE DeviceHandle)
{
    HID_COLLECTION_INFORMATION CollectionInfo = {};
    CollectionInfo.DescriptorSize = sizeof(CollectionInfo);
    DWORD BytesReturned = 0;

    if (DeviceIoControl(
        DeviceHandle,
        IOCTL_HID_GET_COLLECTION_INFORMATION,
        nullptr,
        0,
        &CollectionInfo,
        sizeof(CollectionInfo),
        &BytesReturned,
        nullptr))
    {
        std::cout << "\n  " << "[" << dye::light_purple("\IOCTL_HID_GET_COLLECTION_INFORMATION") << "]" << std::endl;
        
        std::cout << dye::light_purple("    Vendor ID: ") << dye::green("0x") << std::hex << std::uppercase << dye::green(CollectionInfo.VendorID)
            << std::dec << std::nouppercase << std::endl;

        std::cout << dye::light_purple("    Product ID: ") << dye::green("0x") << std::hex << std::uppercase << dye::green(CollectionInfo.ProductID)
            << std::dec << std::nouppercase << std::endl;

        std::cout << dye::light_purple("    Version Number: ") << dye::green("0x") << std::hex << std::uppercase << dye::green(CollectionInfo.VersionNumber)
            << std::dec << std::nouppercase << std::endl;
    }
    else
    {
        // error
    }
}

void HID::GetDriverFromHidDevicePath(const std::wstring& DevicePath)
{
    std::string PathAnsi = UTILS::WStringToString(DevicePath);
    std::cout << "# DevicePath: " << dye::grey(PathAnsi) << std::endl;
}

void HID::ProcessDevice(const std::wstring& DevicePath, const std::wstring& DeviceDescription)
{
    HANDLE DeviceHandle = CreateFileW(
        DevicePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    std::cout << "[" << dye::aqua("HID Device") << "]";
    if (!DeviceDescription.empty())
    {
        std::cout << " - " << UTILS::WStringToString(DeviceDescription);
    }
    std::cout << std::endl;

    GetDriverFromHidDevicePath(DevicePath);

    PrintHidString(DeviceHandle, IOCTL_HID_GET_MANUFACTURER_STRING, "IOCTL_HID_GET_MANUFACTURER_STRING");
    PrintHidString(DeviceHandle, IOCTL_HID_GET_PRODUCT_STRING, "IOCTL_HID_GET_PRODUCT_STRING");
    PrintHidString(DeviceHandle, IOCTL_HID_GET_SERIALNUMBER_STRING, "IOCTL_HID_GET_SERIALNUMBER_STRING");

    PrintCollectionInfo(DeviceHandle);

    std::cout << std::endl;

    CloseHandle(DeviceHandle);
}

void HID::QueryAllInfo()
{
    UTILS::PrintFixedBox("HID Devices");

    GUID HidGuid;
    HidD_GetHidGuid(&HidGuid);

    HDEVINFO DevInfo = SetupDiGetClassDevsW(
        &HidGuid,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (DevInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVICE_INTERFACE_DATA InterfaceData = {};
    InterfaceData.cbSize = sizeof(InterfaceData);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(DevInfo, nullptr, &HidGuid, i, &InterfaceData); i++)
    {
        DWORD RequiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(DevInfo, &InterfaceData, nullptr, 0, &RequiredSize, nullptr);

        if (RequiredSize == 0) continue;

        std::vector<BYTE> Buffer(RequiredSize);
        auto DetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(Buffer.data());
        DetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        SP_DEVINFO_DATA DevInfoData = {};
        DevInfoData.cbSize = sizeof(DevInfoData);

        std::wstring DeviceDescription;

        if (SetupDiGetDeviceInterfaceDetailW(DevInfo, &InterfaceData, DetailData, RequiredSize, nullptr, &DevInfoData))
        {
            WCHAR DescBuffer[256];
            if (SetupDiGetDeviceRegistryPropertyW(DevInfo, &DevInfoData, SPDRP_DEVICEDESC, nullptr, (PBYTE)DescBuffer, sizeof(DescBuffer), nullptr))
            {
                DeviceDescription = DescBuffer;
            }

            ProcessDevice(DetailData->DevicePath, DeviceDescription);
        }
    }

    SetupDiDestroyDeviceInfoList(DevInfo);
}