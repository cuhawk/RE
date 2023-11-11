#include <ntifs.h>
#include <ntddk.h>
#include "ThreadBoostCommon.h"

NTSTATUS ThreadBoostCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ThreadBoostDeviceControl(PDEVICE_OBJECT, PIRP);


void ThreadBoostUnload(PDRIVER_OBJECT DriverObject) {
	KdPrint(("Driver Unload!\n"));
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ThreadBoost");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING /* RegistryPath */) {
//	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("DriverEntry!\n"));

	DriverObject->DriverUnload = ThreadBoostUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = ThreadBoostCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ThreadBoostCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ThreadBoostDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\ThreadBoost");
	//RtlInitUnicodeString(&devName, L"\\Device\\ThreadBoost");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ThreadBoost");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS ThreadBoostCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return STATUS_SUCCESS;
}

NTSTATUS ThreadBoostDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status;
	ULONG len = 0;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_THREAD_BOOST_PRIORITY:
		{
			if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			ThreadData* data = (ThreadData*)Irp->AssociatedIrp.SystemBuffer;
			if (data->Priority < 1 || data->Priority > 31) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			PETHREAD Thread;
			status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
			if (!NT_SUCCESS(status))
				break;

			len = sizeof(ThreadData);
			KeSetPriorityThread((PKTHREAD)Thread, data->Priority);
			ObDereferenceObject(Thread);

			break;
		}

		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, 0);
	return status;
}
