#pragma once

#include <Windows.h>

#define MAX_IDE_DRIVES  16

#define MOUNTMGR_DEVICE_NAME                        L"\\Device\\MountPointManager"
#define MOUNTMGR_DOS_DEVICE_NAME                    L"\\\\.\\MountPointManager"

#define MOUNTMGRCONTROLTYPE                         0x0000006D // 'm'
#define MOUNTDEVCONTROLTYPE                         0x0000004D // 'M'

#define IOCTL_MOUNTDEV_QUERY_UNIQUE_ID CTL_CODE(MOUNTDEVCONTROLTYPE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTMGR_QUERY_POINTS    CTL_CODE(MOUNTMGRCONTROLTYPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_GET_VOLUME_INFORMATION CTL_CODE(IOCTL_DISK_BASE, 0x0019, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _DISK_VOLUME_INFO {
    DWORD SerialNumber;
    DWORD Unknown1;
    DWORD Unknown2;
} DISK_VOLUME_INFO;

typedef struct _MOUNTMGR_MOUNT_POINT {
    ULONG   SymbolicLinkNameOffset;
    USHORT  SymbolicLinkNameLength;
    USHORT  Reserved1;
    ULONG   UniqueIdOffset;
    USHORT  UniqueIdLength;
    USHORT  Reserved2;
    ULONG   DeviceNameOffset;
    USHORT  DeviceNameLength;
    USHORT  Reserved3;
} MOUNTMGR_MOUNT_POINT, * PMOUNTMGR_MOUNT_POINT;

typedef struct _MOUNTMGR_MOUNT_POINTS {
    ULONG                   Size;
    ULONG                   NumberOfMountPoints;
    MOUNTMGR_MOUNT_POINT    MountPoints[1];
} MOUNTMGR_MOUNT_POINTS, * PMOUNTMGR_MOUNT_POINTS;

typedef struct _MOUNTDEV_UNIQUE_ID {
    USHORT UniqueIdLength;
    UCHAR UniqueId[1];
} MOUNTDEV_UNIQUE_ID, * PMOUNTDEV_UNIQUE_ID;