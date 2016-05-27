#include "DeviceIO.h"

#include "../Log/Log.h"
#include "../Notification/Notification.h"

typedef struct _IO_DEVICE_EXTENSION
{
	PVOID ptr;
} IO_DEVICE_EXTENSION, *PIO_DEVICE_EXTENSION;

static UNICODE_STRING uDeviceName;
static UNICODE_STRING uDosDeviceName;
static PDEVICE_OBJECT g_DeviceObject;

_Use_decl_annotations_
NTSTATUS DeviceIOInitialize(
	PDRIVER_OBJECT DriverObject
)
{
	NTSTATUS Status;

	g_DeviceObject = NULL;
	RtlInitUnicodeString(&uDeviceName, DeviceName);
	RtlInitUnicodeString(&uDosDeviceName, DosDeviceName);

	Status = IoCreateDevice(
		DriverObject,
		sizeof(IO_DEVICE_EXTENSION),
		&uDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_UNKNOWN,
		FALSE,
		&g_DeviceObject
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("CreateIODevice: IoCreateDevice failed with error 0x%.8X", Status);
		return Status;
	}

	g_DeviceObject->Flags |= DO_BUFFERED_IO;

	Status = IoCreateSymbolicLink(
		&uDosDeviceName,
		&uDeviceName
	);

	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(g_DeviceObject);
		DEBUG_LOG("CreateIODevice: IoCreateSymbolicLink failed with error 0x%.8X", Status);
		return Status;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE]			= DeviceIOCreate;
	DriverObject->MajorFunction[IRP_MJ_READ]			= DeviceIORead;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]			= DeviceIOCleanup;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]			= DeviceIOClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= DeviceIOControl;


	DEBUG_LOG("CreateIODevice completed");
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS DeviceIOUnload(
	PDRIVER_OBJECT DriverObject
)
{
	IoDeleteSymbolicLink(&uDosDeviceName);
	IoDeleteDevice(g_DeviceObject);
	DEBUG_LOG("DeviceIOUnload completed");
	UNREFERENCED_PARAMETER(DriverObject);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS DeviceIOCreate(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS DeviceIORead(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	PIO_STACK_LOCATION IOStackLocation;
	NTSTATUS Status;
	ULONG BytesRead;

	IOStackLocation = IoGetCurrentIrpStackLocation(Irp);

	BytesRead = 0;

	Status = NotificationCopy(
		Irp->AssociatedIrp.SystemBuffer,
		IOStackLocation->Parameters.Read.Length,
		&BytesRead
	);

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = BytesRead;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	UNREFERENCED_PARAMETER(DeviceObject);
	return Status;
}

_Use_decl_annotations_
NTSTATUS DeviceIOControl(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS DeviceIOCleanup(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS DeviceIOClose(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}
