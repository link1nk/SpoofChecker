# SpoofChecker

> **A tool for verifying hardware IDs, in many different ways.**

**SpoofChecker** is an application developed to help validate the effectiveness of HWID spoofing tools, or simply to allow you to check your HWIDs in a simple way. Unlike basic scripts that only check superficial information, SpoofChecker deeply investigates the system to ensure that identifiers have actually been altered, using ioctl, log reading, PowerShell commands, etc.

Most HWIDs are found more than once using different techniques to ensure they have indeed been spoofed (e.g., Hard Disk), employing several simultaneous extraction methods:

* Reading via Windows APIs (User Mode).
* Direct parsing of Registry structures.
* Low-level reading (Driver/IOCTL/SMART) when available.

## Key Features

* **Disk Analysis:** Physical serials, Volume IDs, MBR/GPT signatures, and SMART data.
* **Network Information:** Current and permanent MAC addresses, ARP cache, and adapter registry keys.
* **SMBIOS & Motherboard:** UUIDs, Serial Numbers, and raw SMBIOS table data.
* **Registry Tracing:** Scanning of keys known to store historical hardware logs (EventLog, MountPoints, etc.).
* **Peripherals:** Identification of connected monitors, keyboards, and USB devices.

## 🤝 Credits

* **[All guys from this thread]** - https://www.unknowncheats.me/forum/anti-cheat-bypass/333662-methods-retrieving-unique-identifiers-hwids-pc.html
* **[Samuel Tulach]** - Thank you for the simple code on how to print the TPM ID. https://github.com/SamuelTulach/tpm-spoofer
* **[Saibulusu]** - Your code about SMBIOS Parser saved me a lot of work, thank you. . https://github.com/saibulusu/SMBIOS-Parser
* **[0mdi]** - I really liked the way you print the serials with the colors, the basic structure of the ioctl is also very good, thank you. https://github.com/0mdi/ultimate_spoofer