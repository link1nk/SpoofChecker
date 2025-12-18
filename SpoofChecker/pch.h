#pragma once

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "rpcrt4.lib")

#define _NTSCSI_USER_MODE_
#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <type_traits>
#include <utility>
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <winioctl.h>
#include <nvme.h>
#include <ntddscsi.h>
#include <ntddstor.h>
#include <locale>
#include <wincrypt.h>
#include <Iphlpapi.h>
#include <ntddndis.h>
#include <filesystem>
#include <sddl.h>
#include <regex>
#include <iosfwd>
#include <string>
#include <fstream>
#include <scsi.h>

#include "Colors.h"
#include "IOCTL_Base.h"
#include "Utils.h"
#include "DiskDefines.h"
#include "Disk.h"
#include "TPM.h"
#include "UsnJrnl.h"
#include "MAC.h"
#include "ARP.h"
#include "VolumesDefines.h"
#include "Volumes.h"