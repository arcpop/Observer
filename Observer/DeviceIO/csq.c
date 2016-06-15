#include "DeviceIO.h"

//Implementation of the cancel safe queue

VOID ObserverCsqInsertIrp(
	_In_ struct _IO_CSQ*	Csq,
	_In_ PIRP				Irp
)
{
	PIO_DEVICE_EXTENSION pDevExt;
	pDevExt = CONTAINING_RECORD(Csq, IO_DEVICE_EXTENSION, CancelSafeQueue);
	InsertTailList(&pDevExt->PendingIRPQueue, &Irp->Tail.Overlay.ListEntry);
}

VOID ObserverCsqRemoveIrp(
	_In_ PIO_CSQ Csq,
	_In_ PIRP    Irp
)
{
	PIO_DEVICE_EXTENSION pDevExt;
	pDevExt = CONTAINING_RECORD(Csq, IO_DEVICE_EXTENSION, CancelSafeQueue);
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

PIRP ObserverCsqPeekNextIrp(
	_In_     PIO_CSQ Csq,
	_In_opt_ PIRP    Irp,
	_In_opt_ PVOID   PeekContext
)
{
	PLIST_ENTRY nextEntry;
	PIO_DEVICE_EXTENSION pDevExt;
	pDevExt = CONTAINING_RECORD(Csq, IO_DEVICE_EXTENSION, CancelSafeQueue);

	if (Irp == NULL)
	{
		nextEntry = pDevExt->PendingIRPQueue.Flink;
	}
	else 
	{
		nextEntry = Irp->Tail.Overlay.ListEntry.Flink;
	}
	if (nextEntry != &pDevExt->PendingIRPQueue)
	{
		return CONTAINING_RECORD(nextEntry, IRP, Tail.Overlay.ListEntry);
	}
	return NULL;
	UNREFERENCED_PARAMETER(PeekContext);
}

VOID ObserverCsqAcquireLock(
	_In_  PIO_CSQ Csq,
	_Out_ PKIRQL  Irql
)
{
	PIO_DEVICE_EXTENSION pDevExt;
	pDevExt = CONTAINING_RECORD(Csq, IO_DEVICE_EXTENSION, CancelSafeQueue);
	KeAcquireSpinLock(&pDevExt->QueueLock, Irql);
}

VOID ObserverCsqReleaseLock(
	_In_ PIO_CSQ Csq,
	_In_ KIRQL   Irql
)
{
	PIO_DEVICE_EXTENSION pDevExt;
	pDevExt = CONTAINING_RECORD(Csq, IO_DEVICE_EXTENSION, CancelSafeQueue);
	KeReleaseSpinLock(&pDevExt->QueueLock, Irql);
}

VOID ObserverCsqCompleteCanceledIrp(
	_In_ PIO_CSQ Csq,
	_In_ PIRP    Irp
)
{
	Irp->IoStatus.Status = STATUS_CANCELLED;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	UNREFERENCED_PARAMETER(Csq);
}

