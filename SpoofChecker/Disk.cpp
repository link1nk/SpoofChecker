#include "pch.h"

#define SENDIDLENGTH sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE

PCHAR DISK::ConvertToString(DWORD* DiskData, ULONG FirstIndex, ULONG LastIndex, PCHAR Buffer)
{
    ULONG Index = 0;
    ULONG Position = 0;

    for (Index = FirstIndex; Index <= LastIndex; Index++)
    {
        Buffer[Position++] = (CHAR)(DiskData[Index] / 256);
        Buffer[Position++] = (CHAR)(DiskData[Index] % 256);
    }

    Buffer[Position] = '\0';

    for (Index = Position - 1; Index > 0 && isspace(Buffer[Index]); Index--)
    {
        Buffer[Index] = '\0';
    }

    return Buffer;
}

void DISK::ConvertToString(WORD* Data, int FirstIndex, int LastIndex, char* OutBuffer)
{
    int k = 0;
    for (int i = FirstIndex; i <= LastIndex; i++)
    {
        OutBuffer[k++] = (char)(Data[i] >> 8);
        OutBuffer[k++] = (char)(Data[i] & 0xFF);
    }
    OutBuffer[k] = '\0';

    for (int i = (int)strlen(OutBuffer) - 1; i >= 0 && OutBuffer[i] == ' '; i--)
    {
        OutBuffer[i] = '\0';
    }
}

void DISK::SwapEndianness(PVOID Ptr, SIZE_T Size)
{
    struct U16Struct
    {
        uint8_t High;
        uint8_t Low;
    };

    for (U16Struct* StructPtr = (U16Struct*)Ptr; StructPtr < (U16Struct*)Ptr + Size / 2; StructPtr++)
    {
        std::swap(StructPtr->Low, StructPtr->High);
    }
}

const char* DISK::IdTypeName(uint8_t IdType)
{
    switch (IdType)
    {
        case 0x01: return "T10 Vendor ID";
        case 0x02: return "EUI-64";
        case 0x03: return "NAA";
        case 0x08: return "NGUID";
        case 0x09: return "NVM Namespace UUID";
        default:   return "Other";
    }
}

std::string DISK::ParseVpd80(const std::vector<uint8_t>& VpdData)
{
    if (VpdData.size() < 4)
    {
        return {};
    }

    uint16_t PageLength = ((uint16_t)VpdData[2] << 8) | VpdData[3];

    if (VpdData.size() < 4 + PageLength)
    {
        return {};
    }

    return UTILS::SafeAscii(VpdData.data() + 4, PageLength);
}

std::vector<DISK::Vpd83Id> DISK::ParseVpd83All(const std::vector<uint8_t>& VpdData)
{
    std::vector<Vpd83Id> Results;

    if (VpdData.size() < 4)
    {
        return Results;
    }

    const size_t PageLength = ((uint16_t)VpdData[2] << 8) | VpdData[3];
    const size_t EndOffset = min(VpdData.size(), (size_t)(4 + PageLength));

    size_t CurrentOffset = 4;

    while (CurrentOffset + 4 <= EndOffset)
    {
        uint8_t ProtoCode = VpdData[CurrentOffset + 0];
        uint8_t PivAssocId = VpdData[CurrentOffset + 1];
        uint8_t Length = VpdData[CurrentOffset + 3];
        size_t NextOffset = CurrentOffset + 4 + Length;

        if (NextOffset > EndOffset)
        {
            break;
        }

        uint8_t CodeSet = ProtoCode & 0x0F;
        uint8_t IdType = PivAssocId & 0x0F;
        const uint8_t* Payload = VpdData.data() + CurrentOffset + 4;

        Vpd83Id IdObj{};
        IdObj.codeSet = CodeSet;
        IdObj.idType = IdType;

        if ((CodeSet == 0x02 || CodeSet == 0x03) && Length > 0)
        {
            IdObj.decoded = UTILS::SafeAscii(Payload, Length);
        }
        else
        {
            IdObj.decoded.clear();
        }

        Results.push_back(std::move(IdObj));
        CurrentOffset = NextOffset;
    }

    return Results;
}

void DISK::PrintVpd83(const std::vector<uint8_t>& VpdData)
{
    auto Ids = ParseVpd83All(VpdData);

    for (size_t Index = 0; Index < Ids.size(); Index++)
    {
        if (!Ids[Index].decoded.empty())
        {
            m_DiskIOCTL.PrintSerial(Ids[Index].decoded, "SCSI_VPD_DEVICE_IDENTIFICATION");
        }
    }
}

HANDLE DISK::GetDevice(ULONG Number, DISK_DEVICE_TYPE Type)
{
    std::string Path;

    switch (Type)
    {
    case DISK_DEVICE_TYPE::PHYSICAL:
    {
        Path = "\\\\.\\PhysicalDrive" + std::to_string(Number);
        break;
    }
    case DISK_DEVICE_TYPE::SCSI:
    {
        Path = "\\\\.\\Scsi" + std::to_string(Number) + ":";
        break;
    }
    }

    m_Handle = CreateFileA(
        Path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (m_Handle != INVALID_HANDLE_VALUE)
    {
        m_Number = Number;
    }

    return m_Handle;
}

void DISK::QuerySmart()
{
    DWORD CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
    auto Command = (SENDCMDINPARAMS*)new UCHAR[CommandSize];

    Command->irDriveRegs.bCommandReg = ID_CMD;

    m_DiskIOCTL.SetIOCTL(m_Handle, SMART_RCV_DRIVE_DATA, "SMART_RCV_DRIVE_DATA");

    if (m_DiskIOCTL.IOCTL(Command, sizeof(SENDCMDINPARAMS), Command, CommandSize))
    {
        DWORD DiskData[256] = { 0 };
        auto* IdSector = (USHORT*)(PIDENTIFY_DATA)((PSENDCMDOUTPARAMS)Command)->bBuffer;

        for (int Index = 0; Index < 256; Index++)
        {
            DiskData[Index] = IdSector[Index];
        }

        char SerialNumber[256] = { 0 };
        ConvertToString(DiskData, 10, 19, SerialNumber);

        m_DiskIOCTL.PrintSerial(SerialNumber);
    }

    delete[] Command;
}

void DISK::QueryStorage()
{
    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_STORAGE_QUERY_PROPERTY, "IOCTL_STORAGE_QUERY_PROPERTY");

    {
        char m_StorageQueryBuf[4096] = { 0 };

        STORAGE_PROPERTY_QUERY Query;
        memset(&Query, 0, sizeof(Query));
        Query.PropertyId = StorageDeviceProperty;
        Query.QueryType = PropertyStandardQuery;
        memset(m_StorageQueryBuf, 0, sizeof(m_StorageQueryBuf));

        m_DiskIOCTL.SetId("StorageDeviceProperty");
        if (m_DiskIOCTL.IOCTL(&Query, sizeof(Query), m_StorageQueryBuf, sizeof(m_StorageQueryBuf)))
        {
            auto Desc = (STORAGE_DEVICE_DESCRIPTOR*)m_StorageQueryBuf;
            auto Serial = ((char*)Desc + Desc->SerialNumberOffset);
            m_DiskIOCTL.PrintSerial(Serial);
        }
    }

    {
        PVOID Buffer = NULL;
        ULONG BufferLength = 0;
        ULONG ReturnedLength = 0;

        PSTORAGE_PROPERTY_QUERY Query = NULL;
        PSTORAGE_PROTOCOL_SPECIFIC_DATA ProtocolData = NULL;
        PSTORAGE_PROTOCOL_DATA_DESCRIPTOR ProtocolDataDescr = NULL;

        BufferLength = FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
        Buffer = malloc(BufferLength);

        ZeroMemory(Buffer, BufferLength);

        Query = (PSTORAGE_PROPERTY_QUERY)Buffer;
        ProtocolDataDescr = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)Buffer;
        ProtocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)Query->AdditionalParameters;

        Query->PropertyId = StorageAdapterProtocolSpecificProperty;
        Query->QueryType = PropertyStandardQuery;

        ProtocolData->ProtocolType = ProtocolTypeNvme;
        ProtocolData->DataType = NVMeDataTypeIdentify;
        ProtocolData->ProtocolDataRequestValue = NVME_IDENTIFY_CNS_CONTROLLER;
        ProtocolData->ProtocolDataRequestSubValue = 0;
        ProtocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        ProtocolData->ProtocolDataLength = NVME_MAX_LOG_SIZE;

        m_DiskIOCTL.SetId("StorageAdapterProtocolSpecificProperty");
        if (m_DiskIOCTL.IOCTL(Buffer, BufferLength, Buffer, BufferLength))
        {
            auto NvmeData = (PNVME_IDENTIFY_CONTROLLER_DATA)((PCHAR)ProtocolData + ProtocolData->ProtocolDataOffset);
            std::string Serial((char*)NvmeData->SN, 20);
            m_DiskIOCTL.PrintSerial(Serial);
        }

        free(Buffer);
    }

    {
        PVOID Buffer = nullptr;
        ULONG BufferLength = 0;
        ULONG ReturnedLength = 0;

        PSTORAGE_PROPERTY_QUERY Query = NULL;
        PSTORAGE_PROTOCOL_SPECIFIC_DATA ProtocolData = NULL;
        PSTORAGE_PROTOCOL_DATA_DESCRIPTOR ProtocolDataDescr = NULL;

        BufferLength = FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
        Buffer = malloc(BufferLength);

        if (Buffer == nullptr)
        {
            return;
        }

        ZeroMemory(Buffer, BufferLength);

        Query = (PSTORAGE_PROPERTY_QUERY)Buffer;
        ProtocolDataDescr = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)Buffer;
        ProtocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)Query->AdditionalParameters;

        Query->PropertyId = StorageDeviceProtocolSpecificProperty;
        Query->QueryType = PropertyStandardQuery;

        ProtocolData->ProtocolType = ProtocolTypeNvme;
        ProtocolData->DataType = NVMeDataTypeIdentify;
        ProtocolData->ProtocolDataRequestValue = NVME_IDENTIFY_CNS_CONTROLLER;
        ProtocolData->ProtocolDataRequestSubValue = 0;
        ProtocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        ProtocolData->ProtocolDataLength = NVME_MAX_LOG_SIZE;

        m_DiskIOCTL.SetId("StorageDeviceProtocolSpecificProperty");
        if (m_DiskIOCTL.IOCTL(Buffer, BufferLength, Buffer, BufferLength))
        {
            auto NvmeData = (PNVME_IDENTIFY_CONTROLLER_DATA)((PCHAR)ProtocolData + ProtocolData->ProtocolDataOffset);
            std::string Serial((char*)NvmeData->SN, 20);
            m_DiskIOCTL.PrintSerial(Serial);
        }

        free(Buffer);
    }
}

void DISK::QueryAtaPassThrough()
{
    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_ATA_PASS_THROUGH, "IOCTL_ATA_PASS_THROUGH");

    CHAR Buffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };
    ATA_PASS_THROUGH_EX& PteInfo = *(ATA_PASS_THROUGH_EX*)Buffer;
    IDEREGS* IdeRegs = (IDEREGS*)PteInfo.CurrentTaskFile;
    DWORD Bytes = 0;

    PteInfo.AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED;
    PteInfo.Length = sizeof(PteInfo);
    PteInfo.DataTransferLength = 512;
    PteInfo.TimeOutValue = 10;
    PteInfo.DataBufferOffset = sizeof(PteInfo);

    IdeRegs->bCommandReg = IDE_COMMAND_IDENTIFY;

    if (m_DiskIOCTL.IOCTL(&PteInfo, sizeof(PteInfo), Buffer, sizeof(Buffer)))
    {
        auto DiskInfo = (PIDENTIFY_DEVICE_DATA)(Buffer + sizeof(PteInfo));

        SwapEndianness(DiskInfo->SerialNumber, sizeof(DiskInfo->SerialNumber));
        m_DiskIOCTL.PrintSerial((PCHAR)DiskInfo->SerialNumber);
    }
}

void DISK::QueryScsiMiniport()
{
    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_SCSI_MINIPORT, "IOCTL_SCSI_MINIPORT");

    for (ULONG DriveIndex = 0; DriveIndex < 2; DriveIndex++)
    {
        char Buffer[sizeof(SRB_IO_CONTROL) + SENDIDLENGTH];

        SRB_IO_CONTROL* SrbControl = (SRB_IO_CONTROL*)Buffer;
        SENDCMDINPARAMS* CmdInParams = (SENDCMDINPARAMS*)(Buffer + sizeof(SRB_IO_CONTROL));

        memset(Buffer, 0, sizeof(Buffer));
        SrbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
        SrbControl->Timeout = 10000;
        SrbControl->Length = SENDIDLENGTH;
        SrbControl->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
        strncpy((char*)SrbControl->Signature, "SCSIDISK", 8);

        CmdInParams->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
        CmdInParams->bDriveNumber = (BYTE)DriveIndex;

        if (m_DiskIOCTL.IOCTL(Buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1, Buffer, sizeof(SRB_IO_CONTROL) + SENDIDLENGTH))
        {
            SENDCMDOUTPARAMS* CmdOutParams = (SENDCMDOUTPARAMS*)(Buffer + sizeof(SRB_IO_CONTROL));
            IDSECTOR* IdSector = (IDSECTOR*)(CmdOutParams->bBuffer);

            char SerialNumber[64];
            ConvertToString((WORD*)IdSector, 10, 19, SerialNumber);

            if (SerialNumber[0] != '\0')
            {
                m_DiskIOCTL.PrintSerial(SerialNumber, (DriveIndex == 0 ? "MASTER" : "SLAVE"));
            }
        }
    }
}

void DISK::QueryNvmeIdentify()
{
    const ULONG ByteSizeTX = 4096;
    ULONG TotalBufferSize = offsetof(NVME_PASS_THROUGH_IOCTL, DataBuffer) + ByteSizeTX;

    std::vector<char> RawBuffer(TotalBufferSize, 0);
    NVME_PASS_THROUGH_IOCTL* PassThrough = reinterpret_cast<NVME_PASS_THROUGH_IOCTL*>(RawBuffer.data());

    PassThrough->SrbIoCtrl.HeaderLength = sizeof(SRB_IO_CONTROL);
    memcpy(PassThrough->SrbIoCtrl.Signature, "NvmeMini", 8);
    PassThrough->SrbIoCtrl.Timeout = 60;
    PassThrough->SrbIoCtrl.ControlCode = NVME_PASS_THROUGH_SRB_IO_CODE;
    PassThrough->SrbIoCtrl.Length = TotalBufferSize - sizeof(SRB_IO_CONTROL);

    PassThrough->NVMeCmd[0] = 0x06;
    PassThrough->NVMeCmd[1] = 0;
    PassThrough->NVMeCmd[10] = 1;

    PassThrough->Direction = NVME_FROM_DEV_TO_HOST;
    PassThrough->DataBufferLen = ByteSizeTX;
    PassThrough->ReturnBufferLen = TotalBufferSize;

    DWORD Count = 0;
    BOOL Status = DeviceIoControl(
        m_Handle,
        IOCTL_SCSI_MINIPORT,
        PassThrough, TotalBufferSize,
        PassThrough, TotalBufferSize,
        &Count, NULL);

    if (Status && ((PassThrough->CplEntry[3] >> 17) & 0xFF) == 0)
    {
        std::cout << "SUCESSO! NVMe Identify obtido." << std::endl;

        char SerialNum[21] = { 0 };
        memcpy(SerialNum, &PassThrough->DataBuffer[4], 20);
        for (int K = 0; K < 20; K += 2) std::swap(SerialNum[K], SerialNum[K + 1]);
        std::cout << "SN: " << SerialNum << std::endl;
    }
}

void DISK::QueryScsiPassThrough(UCHAR PageCode)
{
    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_SCSI_PASS_THROUGH, "IOCTL_SCSI_PASS_THROUGH");

    std::vector<uint8_t> VpdData;

    struct PacketStruct
    {
        SCSI_PASS_THROUGH Spt;
        ULONG Filler;
        UCHAR Sense[32];
        UCHAR Data[0x1000];
    } Packet{};

    ZeroMemory(&Packet, sizeof(Packet));

    Packet.Spt.Length = sizeof(SCSI_PASS_THROUGH);
    Packet.Spt.CdbLength = 6;
    Packet.Spt.SenseInfoLength = sizeof(Packet.Sense);
    Packet.Spt.DataIn = SCSI_IOCTL_DATA_IN;
    Packet.Spt.DataTransferLength = sizeof(Packet.Data);
    Packet.Spt.TimeOutValue = 5;
    Packet.Spt.DataBufferOffset = offsetof(PacketStruct, Data);
    Packet.Spt.SenseInfoOffset = offsetof(PacketStruct, Sense);

    Packet.Spt.Cdb[0] = 0x12;
    Packet.Spt.Cdb[1] = 0x01;
    Packet.Spt.Cdb[2] = PageCode;
    Packet.Spt.Cdb[3] = 0x00;
    Packet.Spt.Cdb[4] = (UCHAR)std::min<size_t>(sizeof(Packet.Data), 0xFF);
    Packet.Spt.Cdb[5] = 0x00;

    if (m_DiskIOCTL.IOCTL(&Packet, sizeof(Packet), &Packet, sizeof(Packet)))
    {
        UCHAR* DataPtr = Packet.Data;

        if (Packet.Spt.DataTransferLength < 4) return;
        if (DataPtr[1] != PageCode) return;

        uint16_t PageLength = ((uint16_t)DataPtr[2] << 8) | DataPtr[3];
        size_t TotalSize = 4 + PageLength;
        if (TotalSize > sizeof(Packet.Data)) TotalSize = sizeof(Packet.Data);

        VpdData.assign(DataPtr, DataPtr + TotalSize);

        if (PageCode == 0x80)
        {
            std::string Serial80 = ParseVpd80(VpdData);
            if (!Serial80.empty())
            {
                m_DiskIOCTL.PrintSerial(Serial80, "SCSI_VPD_UNIT_SERIAL_NUMBER");
            }
        }
        else if (PageCode == 0x83)
        {
            PrintVpd83(VpdData);
        }
    }
}

void DISK::QueryScsiPassThroughDirect(UCHAR PageCode)
{
    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, "IOCTL_SCSI_PASS_THROUGH_DIRECT");

    std::vector<uint8_t> VpdData;

    std::vector<UCHAR> DataBuffer(0x1000);
    ZeroMemory(DataBuffer.data(), DataBuffer.size());

    struct PacketDirectStruct
    {
        SCSI_PASS_THROUGH_DIRECT Sptd;
        ULONG Filler;
        UCHAR Sense[32];
    } Packet{};

    ZeroMemory(&Packet, sizeof(Packet));

    Packet.Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    Packet.Sptd.CdbLength = 6;
    Packet.Sptd.SenseInfoLength = sizeof(Packet.Sense);
    Packet.Sptd.DataIn = SCSI_IOCTL_DATA_IN;
    Packet.Sptd.DataTransferLength = (ULONG)DataBuffer.size();
    Packet.Sptd.TimeOutValue = 5;

    Packet.Sptd.DataBuffer = DataBuffer.data();
    Packet.Sptd.SenseInfoOffset = offsetof(PacketDirectStruct, Sense);

    Packet.Sptd.Cdb[0] = 0x12;
    Packet.Sptd.Cdb[1] = 0x01;
    Packet.Sptd.Cdb[2] = PageCode;
    Packet.Sptd.Cdb[3] = 0x00;
    Packet.Sptd.Cdb[4] = (UCHAR)std::min<size_t>(DataBuffer.size(), 0xFF);
    Packet.Sptd.Cdb[5] = 0x00;

    if (m_DiskIOCTL.IOCTL(&Packet, sizeof(Packet), &Packet, sizeof(Packet)))
    {
        if (Packet.Sptd.ScsiStatus != 0)
        {
            return;
        }

        UCHAR* DataPtr = DataBuffer.data();

        if (Packet.Sptd.DataTransferLength < 4) return;
        if (DataPtr[1] != PageCode) return;

        uint16_t PageLength = ((uint16_t)DataPtr[2] << 8) | DataPtr[3];
        size_t TotalSize = 4 + PageLength;
        if (TotalSize > DataBuffer.size()) TotalSize = DataBuffer.size();

        VpdData.assign(DataPtr, DataPtr + TotalSize);

        if (PageCode == 0x80)
        {
            std::string Serial80 = ParseVpd80(VpdData);
            if (!Serial80.empty())
                m_DiskIOCTL.PrintSerial(Serial80, "SCSI_VPD_UNIT_SERIAL_NUMBER");
        }
        else if (PageCode == 0x83)
        {
            PrintVpd83(VpdData);
        }
    }
}

void DISK::QueryDiskId()
{
    BYTE Buffer[1024] = { 0 };

    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, "IOCTL_DISK_GET_DRIVE_LAYOUT_EX");

    if (m_DiskIOCTL.IOCTL(nullptr, 0, Buffer, sizeof(Buffer)))
    {
        DRIVE_LAYOUT_INFORMATION_EX* Layout = reinterpret_cast<DRIVE_LAYOUT_INFORMATION_EX*>(Buffer);

        if (Layout->PartitionStyle == PARTITION_STYLE_GPT)
        {
            m_DiskIOCTL.PrintGUID(Layout->Gpt.DiskId);
        }
        else if (Layout->PartitionStyle == PARTITION_STYLE_MBR)
        {
            printf("DISK Signature (MBR): %08X\n", Layout->Mbr.Signature);
        }
        else
        {
            std::cout << "Estilo de partição desconhecido." << std::endl;
        }
    }
}

void DISK::QueryPartitionInfo()
{
    PARTITION_INFORMATION_EX PartInfo = {};

    m_DiskIOCTL.SetIOCTL(m_Handle, IOCTL_DISK_GET_PARTITION_INFO_EX, "IOCTL_DISK_GET_PARTITION_INFO_EX");

    if (m_DiskIOCTL.IOCTL(nullptr, 0, &PartInfo, sizeof(PartInfo)))
    {
        switch (PartInfo.PartitionStyle)
        {
        case PARTITION_STYLE_MBR:
            m_DiskIOCTL.PrintGUID(PartInfo.Mbr.PartitionId);
            break;

        case PARTITION_STYLE_GPT:
            m_DiskIOCTL.PrintGUID(PartInfo.Gpt.PartitionId);
            break;

        case PARTITION_STYLE_RAW:
            std::wcout << L"RAW (without valid partition)\n";
            break;
        }
    }
}

void DISK::QueryPowershellCommandInfo()
{
    std::string RawPhysical = UTILS::RunPowerShell(L"Get-PhysicalDisk | Select-Object -ExpandProperty SerialNumber");
    std::vector<std::string> SerialsPhysical = UTILS::SplitLines(RawPhysical);

    std::cout << "[" << dye::light_purple("Get-PhysicalDisk") << "]\n";
    for (size_t Index = 0; Index < SerialsPhysical.size(); Index++)
    {
        std::cout << "Disk " << Index << " - Serial: " << dye::green(SerialsPhysical[Index]) << std::endl;
    }

    std::cout << std::endl;

    std::string RawDisk = UTILS::RunPowerShell(
        L"Get-Disk | ForEach-Object { "
        L"$disk = $_; "
        L"$wmi = Get-WmiObject Win32_DiskDrive | Where-Object { $_.Index -eq $disk.Number }; "
        L"$wmi.SerialNumber "
        L"}"
    );
    std::vector<std::string> SerialsDisk = UTILS::SplitLines(RawDisk);

    std::cout << "[" << dye::light_purple("Get-Disk") << "]\n";
    for (size_t Index = 0; Index < SerialsDisk.size(); Index++)
    {
        std::cout << "Disk " << Index << " - Serial: " << dye::green(SerialsDisk[Index]) << std::endl;
    }

    std::cout << std::endl;
}

void DISK::QueryAllInfo()
{
    UTILS::PrintFixedBox("Physical Drive");

    for (ULONG Index = 0; Index < MAX_IDE_DRIVES; Index++)
    {
        m_Handle = GetDevice(Index, DISK_DEVICE_TYPE::PHYSICAL);

        if (IsValid())
        {
            std::cout << "# DISK " << m_Number << std::endl;

            QuerySmart();
            QueryAtaPassThrough();
            QueryStorage();
            QueryDiskId();
            QueryPartitionInfo();
            QueryScsiPassThrough(SCSI_VPD_UNIT_SERIAL_NUMBER);
            QueryScsiPassThrough(SCSI_VPD_DEVICE_IDENTIFICATION);
            QueryScsiPassThroughDirect(SCSI_VPD_UNIT_SERIAL_NUMBER);
            QueryScsiPassThroughDirect(SCSI_VPD_DEVICE_IDENTIFICATION);

            std::cout << std::endl;
        }
    }

    QueryPowershellCommandInfo();

    UTILS::PrintFixedBox("SCSI");

    for (ULONG Index = 0; Index < MAX_IDE_DRIVES; Index++)
    {
        m_Handle = GetDevice(Index, DISK_DEVICE_TYPE::SCSI);

        if (IsValid())
        {
            std::cout << "# SCSI " << m_Number << std::endl;

            QueryStorage();
            QueryScsiMiniport();

            std::cout << std::endl;
        }
    }
}