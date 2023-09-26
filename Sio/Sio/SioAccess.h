
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <wdmsec.h> // for SDDLs
#include "Trace.h" // contains macros for WPP tracing

#define NTDEVICE_NAME_STRING      L"\\Device\\SIOACC"
#define SYMBOLIC_NAME_STRING     L"\\DosDevices\\SIOACC"

#define SIO_INDEX_PORT 0x2E
#define SIO_DATA_PORT 0x2F
#define SEC_SIO_INDEX_PORT 0x4E
#define SEC_SIO_DATA_PORT 0x4F
#define SIO_HWM_PORT_0 0x290
#define SIO_HWM_PORT_1 0x2A0
#define SIO_HWM_PORT_2 0xA20

#define IOCTL_CUSTOM_READ_IO_COMMAND  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CUSTOM_WRITE_IO_COMMAND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _IO_PORT_INPUT {
    ULONG PortNumber;
    union _IO_PORT_DATA {
        ULONG  LongData;
        USHORT ShortData;
        UCHAR  CharData;
    } IoPortData;
} IO_PORT_INPUT, * PIO_PORT_INPUT;


typedef struct _CONTROL_DEVICE_EXTENSION {

    HANDLE   FileHandle; // Store your control data here

} CONTROL_DEVICE_EXTENSION, * PCONTROL_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(CONTROL_DEVICE_EXTENSION,
    ControlGetData)

    //
    // Device driver routine declarations.
    //

    DRIVER_INITIALIZE DriverEntry;

//
// Don't use EVT_WDF_DRIVER_DEVICE_ADD for NonPnpDeviceAdd even though 
// the signature is same because this is not an event called by the 
// framework.
//
NTSTATUS
SioAccDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
);

EVT_WDF_DRIVER_UNLOAD SioAccEvtDriverUnload;

EVT_WDF_DEVICE_CONTEXT_CLEANUP SioAccEvtDriverContextCleanup;
EVT_WDF_DEVICE_SHUTDOWN_NOTIFICATION SioAccShutdown;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SioAccEvtIoDeviceControl;

EVT_WDF_DEVICE_FILE_CREATE SioAccEvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE SioAccEvtFileClose;

NTSTATUS
ReadIoPort(
    IN WDFREQUEST Request,
    IN ULONG IoControlCode
);

NTSTATUS
WriteIoPort(
    IN WDFREQUEST Request,
    IN ULONG IoControlCode
);

#pragma warning(disable:4127)

