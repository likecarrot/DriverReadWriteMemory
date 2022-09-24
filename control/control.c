#include    <stdio.h>
#include    <Windows.h>



#define	READ_MODE	0
#define	WRITE_MODE	1

#define	IOCTL_OPREADWRITE CTL_CODE(FILE_DEVICE_UNKNOWN,0x903,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	DevName L"\\??\\Device\\OPReadWrite"


typedef struct {
    HANDLE	pid;
    PVOID	addr;
    PVOID	buf;
    SIZE_T	size;
    ULONG	mode;
}READWRITE_STRUCT, * PREADWRITE_STRUCT;

int main()
{
    HANDLE  hDevice = NULL;
    READWRITE_STRUCT str = { 0 };
    str.size = 4;
    str.addr = (PVOID)0x276BF2DCE3C;
    str.pid = (HANDLE)15176;
    CHAR    buffer[32];
    str.buf = buffer;
    str.mode = READ_MODE;
    hDevice=CreateFile(DevName, GENERIC_ALL, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice==INVALID_HANDLE_VALUE)
    {
        printf("CreateFile ==INVALID_HANDLE_VALUE \n");
    }
    BOOL  retstatus;
    retstatus = DeviceIoControl(hDevice, IOCTL_OPREADWRITE, &str, sizeof(str), NULL, NULL, NULL, NULL);
    if (retstatus)
    {
        printf("DeviceIoControl-%x\n",str.buf);
    }
    system("pause");
    return 0;
}
