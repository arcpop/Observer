#ifndef DEVICE_IO_H
#define DEVICE_IO_H

#pragma once

#include <ntddk.h>

BOOLEAN VerifyProcess(PEPROCESS Process);

NTSTATUS  DeviceIOInitialize(
	_In_ PDRIVER_OBJECT DriverObject
);

NTSTATUS  DeviceIOUnload(
	_In_ PDRIVER_OBJECT DriverObject
);

_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH  DeviceIOCreate;
_Dispatch_type_(IRP_MJ_READ)
DRIVER_DISPATCH  DeviceIORead;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH  DeviceIOControl;
_Dispatch_type_(IRP_MJ_CLEANUP)
DRIVER_DISPATCH  DeviceIOCleanup;
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH  DeviceIOClose;


typedef struct _IO_DEVICE_EXTENSION
{
	IO_CSQ			CancelSafeQueue;
	LIST_ENTRY		PendingIRPQueue;
	KSPIN_LOCK		QueueLock;
} IO_DEVICE_EXTENSION, *PIO_DEVICE_EXTENSION;

IO_CSQ_INSERT_IRP            ObserverCsqInsertIrp;
IO_CSQ_REMOVE_IRP            ObserverCsqRemoveIrp;
IO_CSQ_PEEK_NEXT_IRP         ObserverCsqPeekNextIrp;
IO_CSQ_ACQUIRE_LOCK          ObserverCsqAcquireLock;
IO_CSQ_RELEASE_LOCK          ObserverCsqReleaseLock;
IO_CSQ_COMPLETE_CANCELED_IRP ObserverCsqCompleteCanceledIrp;

#define DEVICEIO_TAG 'OIbO'
#define DEVICEIO_ALLOCATE(size, type) ExAllocatePoolWithTag(type, size, DEVICEIO_TAG)
#define DEVICEIO_FREE(ptr) ExFreePoolWithTag(ptr, DEVICEIO_TAG)

#endif // !DEVICE_IO_H
