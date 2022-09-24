#define	READ_MODE	0
#define	WRITE_MODE	1

VOID	DriverUnload(PDRIVER_OBJECT pDriver);
NTSTATUS	OPDispatch(PDEVICE_OBJECT pDevice, PIRP irp);
NTSTATUS	OPCreateDevice(PDEVICE_OBJECT pDevice, PIRP irp);
NTSTATUS	OPControlDevice(PDEVICE_OBJECT pDevice, PIRP irp);

#define	IOCTL_OPREADWRITE CTL_CODE(FILE_DEVICE_UNKNOWN,0x903,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	DevName2 L"\\Device\\OPReadWrite"


typedef struct {
	HANDLE	pid;
	PVOID	addr;
	PVOID	buf;
	SIZE_T	size;
	ULONG	mode;
}READWRITE_STRUCT,*PREADWRITE_STRUCT;