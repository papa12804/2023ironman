#include <stdio.h>
#include <stdlib.h>
#include <Shlwapi.h>
#include <strsafe.h>
#pragma warning(disable:4201)  // nameless struct/union
#include <winioctl.h>
#pragma warning(default:4201)
#include <cfgmgr32.h>
#include <initguid.h>

#pragma once
#ifdef SIOSDK_EXPORTS
#define SIO_API __declspec(dllexport)
#else
#define SIO_API __declspec(dllimport)
#endif

#define SVCNAME TEXT("SIOACC")
#define DEVICE_NAME TEXT("\\\\.\\SIOACC")

#define INDEX_PORT     0x2E
#define DATA_PORT      0x2F
#define SEC_INDEX_PORT 0x4E
#define SEC_DATA_PORT  0x4F
#define ENTER_CONFIG   0x87
#define EXIT_CONFIG    0xAA
#define LDN_SEL        0x07
#define LDN_HWM        0x0B
#define BASE1_LO       0x61
#define BASE1_HI       0x60
#define HWM_BANK_NO    0x4E

#define IOCTL_CUSTOM_READ_IO_COMMAND  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CUSTOM_WRITE_IO_COMMAND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _IO_PORT_INPUT {
	ULONG PortNumber;
	union _IO_PORT_DATA {
		ULONG  LongData;
		USHORT ShortData;
		UCHAR  CharData;
	} IoPortData;
} IO_PORT_INPUT;

extern "C"
{
	SIO_API HANDLE hDevice;
	SIO_API bool Install();
	SIO_API bool Open();
	SIO_API bool Close();
	SIO_API unsigned int GetCpuFanSpeed();
}
