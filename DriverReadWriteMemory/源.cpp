#include	<ntifs.h>
#include	"header.h"


PDRIVER_OBJECT	g_pdriver = NULL;

NTSTATUS	ReadWriteProcess_Mdl(HANDLE pid, PVOID addr, PVOID buf, SIZE_T size, ULONG mode) {
	PEPROCESS	pe = NULL;
	if (!NT_SUCCESS(PsLookupProcessByProcessId(pid,&pe)))
	{
		return	STATUS_UNSUCCESSFUL;
	}
	ObDereferenceObject(pe);
	KAPC_STATE	kapc = { 0 };
	KeStackAttachProcess(pe, &kapc);
	PMDL	mdl = IoAllocateMdl(addr, size, FALSE, FALSE, NULL);
	if (!mdl)
	{
		KeUnstackDetachProcess(&kapc);
		return	STATUS_UNSUCCESSFUL;
	}
	__try {
		MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	}
	__except (1) {
		KeUnstackDetachProcess(&kapc);
		IoFreeMdl(mdl);
		return	STATUS_UNSUCCESSFUL;
	}
	KeUnstackDetachProcess(&kapc);

	PVOID	map = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmCached, NULL, FALSE,HighPagePriority);
	if (!map)
	{
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
		return	STATUS_UNSUCCESSFUL;
	}
	if (mode==READ_MODE)
	{
		RtlCopyMemory(buf, map, size);
	}
	else
	{
		RtlCopyMemory(map, buf, size);
	}
	MmUnmapLockedPages(map, mdl);
	MmUnlockPages(mdl);
	IoFreeMdl(mdl);
	return	STATUS_SUCCESS;
}


EXTERN_C	NTSTATUS	
DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegistry) {
	UNICODE_STRING	DevName = { 0 };
	PDEVICE_OBJECT	pDevice = NULL;
	RtlInitUnicodeString(&DevName, L"\\Device\\OPReadWrite");
	NTSTATUS	status = NULL;
	g_pdriver = pDriver;
	pDriver->DriverUnload = DriverUnload;
	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriver->MajorFunction[i] = OPDispatch;
	}
	pDriver->MajorFunction[IRP_MJ_CREATE] = OPCreateDevice;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OPControlDevice;
	status=IoCreateDevice(pDriver, NULL,&DevName,FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,FALSE,&pDevice);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("OPReadWrite:DriverEntry-status-%x", status));
	}
	KdPrint(("OPReadWrite:DriverEntry-status-%x", status));
	return status;
}
VOID	DriverUnload(PDRIVER_OBJECT pDriver) {
	if (pDriver->DeviceObject!=NULL)
	{
		IoDeleteDevice(pDriver->DeviceObject);
		KdPrint(("OPReadWrite:DriverUnload-delete-device"));
	}
}
NTSTATUS	OPDispatch(PDEVICE_OBJECT pDevice, PIRP irp) {
	UNREFERENCED_PARAMETER(pDevice);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}
NTSTATUS	OPCreateDevice(PDEVICE_OBJECT pDevice, PIRP irp) {
	UNREFERENCED_PARAMETER(pDevice);
	KdPrint(("OPCreateDevice:create-device"));
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}
NTSTATUS	OPControlDevice(PDEVICE_OBJECT pDevice, PIRP irp) {
	UNREFERENCED_PARAMETER(pDevice);
	KdPrint(("OPControlDevice:Control-device"));
	PIO_STACK_LOCATION irpsp= IoGetCurrentIrpStackLocation(irp);
	if (irpsp->MajorFunction==IRP_MJ_DEVICE_CONTROL)
	{
		PREADWRITE_STRUCT myStr = (PREADWRITE_STRUCT)irp->AssociatedIrp.SystemBuffer;
		ReadWriteProcess_Mdl(myStr->pid, myStr->addr,myStr->buf, myStr->size, myStr->mode);
	}
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}