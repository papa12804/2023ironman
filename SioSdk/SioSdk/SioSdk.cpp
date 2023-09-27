#include "pch.h"
#include "SioSdk.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

unsigned char ReadIoPort(unsigned int Index);
bool WriteIoPort(unsigned int Index, unsigned int Data);
unsigned char ReadSio(unsigned int Index);
void WriteSio(unsigned int Index, unsigned int Data);
void OpenSioConfig();
void CloseSioConfig();
unsigned int GetHwmBase();
unsigned char ReadHwm(unsigned int Base, unsigned int Bank, unsigned int Register);
bool WriteHwm(unsigned int Base, unsigned int Bank, unsigned int Register, unsigned int Data);

bool Install()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD err;
    BOOL ok;
    bool Result = FALSE;
    TCHAR szPath[MAX_PATH];
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return Result;
    }
    PathRemoveFileSpec(szUnquotedPath);
    StringCbCat(szUnquotedPath, MAX_PATH, TEXT("\\resources\\lib\\Sio.sys"));
    StringCbPrintf(szPath, MAX_PATH, TEXT("%s"), szUnquotedPath);
    printf("System file path: %S\n", szPath);

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return Result;
    }
    else
    {
        printf("OpenSCManager successfully\n");
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_KERNEL_DRIVER,     // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        err = GetLastError();
        if (err != ERROR_SERVICE_EXISTS) {
            printf("CreateService failed (%d)\n", err);
            CloseServiceHandle(schSCManager);
            return Result;
        }
    }
    else
    {
        Result = TRUE;
        printf("CreateService successfully\n");
    }

    schService = OpenService(schSCManager, SVCNAME, SERVICE_ALL_ACCESS);
    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        Result = FALSE;
        return Result;
    }

    ok = StartService(schService, 0, NULL);
    if (ok)
    {
        Result = TRUE;
        printf("StartService successfully\n");
    }
    else
    {
        err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            printf("StartService failed (%d)\n", err);
            Result = FALSE;
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return Result;
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return Result;
}

bool Open()
{
    bool Result = FALSE;

    hDevice = CreateFile(DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed (%d)\n", GetLastError());
        return Result;
    }
    else
    {
        Result = TRUE;
        printf("CreateFile successfully\n");
    }

    return Result;
}

bool Close()
{
    bool Result = FALSE;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS Status;

    if (hDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDevice);
        Result = TRUE;
    }

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        printf("Manager is not existed\n");
        Result = FALSE;
        return Result;
    }

    schService = OpenService(schSCManager, SVCNAME, SERVICE_ALL_ACCESS);
    if (schService == NULL)
    {
        Result = TRUE;
        printf("Service is not existed\n");
        CloseServiceHandle(schSCManager);
        return Result;
    }

    ControlService(schService, SERVICE_CONTROL_STOP, &Status);
    DeleteService(schService);
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    printf("Service closed successfully\n");

    return Result;
}

unsigned char ReadIoPort(unsigned int Index)
{
    bool Result = FALSE;
    unsigned char Output;
    unsigned long bytesReturned;
    IO_PORT_INPUT Input;

    printf("ReadIoPort\n");

    Input.PortNumber = Index;
    Result = (BOOLEAN)DeviceIoControl(hDevice,
        (DWORD)IOCTL_CUSTOM_READ_IO_COMMAND,
        &Input,
        sizeof(Input),
        &Output,
        sizeof(Output),
        &bytesReturned,
        NULL
    );
    printf("Read status %d\n", Result);
    printf("Output %x\n", Output);
    printf("bytesReturned %x\n", bytesReturned);

    return Output;
}

bool WriteIoPort(unsigned int Index, unsigned int Data)
{
    bool Result = FALSE;
    IO_PORT_INPUT Input;
    unsigned char Output;

    printf("WriteIoPort\n");

    Input.PortNumber = Index;
    Input.IoPortData.CharData = (unsigned char)Data;
    Result = (BOOLEAN)DeviceIoControl(hDevice,
        (DWORD)IOCTL_CUSTOM_WRITE_IO_COMMAND,
        &Input,
        sizeof(Input),
        &Output,
        sizeof(Output),
        NULL,
        NULL
    );
    printf("Write status %d\n", Result);

    return Result;
}

unsigned char ReadSio(unsigned int Index)
{
    unsigned int IndexPort = INDEX_PORT;
    unsigned int DataPort = DATA_PORT;
    unsigned char Output;

    printf("ReadSio\n");
    WriteIoPort(IndexPort, Index);
    Output = ReadIoPort(DataPort);

    return Output;
}

void WriteSio(unsigned int Index, unsigned int Data)
{
    unsigned int IndexPort = INDEX_PORT;
    unsigned int DataPort = DATA_PORT;
    bool Result = FALSE;

    printf("WriteSio\n");
    Result = WriteIoPort(IndexPort, Index);
    Result = WriteIoPort(DataPort, Data);

    return;
}

void OpenSioConfig()
{
    unsigned int Index = INDEX_PORT;

    printf("OpenSioConfig\n");
    WriteIoPort(Index, ENTER_CONFIG);
    WriteIoPort(Index, ENTER_CONFIG);
    return;
}

void CloseSioConfig()
{
    unsigned int Index = INDEX_PORT;

    printf("CloseSioConfig\n");
    WriteIoPort(Index, EXIT_CONFIG);
    return;
}

unsigned int GetHwmBase()
{
    unsigned int IndexPort = INDEX_PORT;
    unsigned int DataPort = DATA_PORT;
    unsigned int BaseLow, BaseHigh;

    printf("GetHwmBase\n");
    WriteSio(LDN_SEL, LDN_HWM);
    BaseLow = (unsigned int)ReadSio(BASE1_LO) & 0xFF;
    BaseHigh = (unsigned int)ReadSio(BASE1_HI) & 0xFF;

    return (BaseHigh << 8) + BaseLow;
}

unsigned char ReadHwm(unsigned int Base, unsigned int Bank, unsigned int Register)
{
    unsigned char Temp;
    unsigned char Output;

    printf("ReadHwm\n");
    WriteIoPort(Base + 0x05, HWM_BANK_NO);
    Temp = ReadIoPort(Base + 0x06);
    Temp &= 0xF0;
    WriteIoPort(Base + 0x06, Temp | Bank);

    WriteIoPort(Base + 0x05, Register);
    Output = ReadIoPort(Base + 0x06);

    return Output;
}

bool WriteHwm(unsigned int Base, unsigned int Bank, unsigned int Register, unsigned int Data)
{
    unsigned char Temp;
    bool Result = FALSE;

    printf("WriteHwm\n");
    WriteIoPort(Base + 0x05, HWM_BANK_NO);
    Temp = ReadIoPort(Base + 0x06);
    Temp &= 0xF0;
    WriteIoPort(Base + 0x06, Temp | Bank);

    Result = WriteIoPort(Base + 0x05, Register);
    Result = WriteIoPort(Base + 0x06, Data);

    return Result;
}

unsigned int GetCpuFanSpeed()
{
    unsigned int Base;
    unsigned int RpmLow, RpmHigh;

    printf("GetCpuFanSpeed\n");
    OpenSioConfig();
    Base = GetHwmBase();
    CloseSioConfig();
    RpmHigh = ReadHwm(Base, 0x04, 0xC2);
    RpmLow = ReadHwm(Base, 0x04, 0xC3);

    return (RpmHigh << 8) + RpmLow;
}
