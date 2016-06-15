#include "NotificationQueue.h"

FAST_MUTEX	NotificationListMutex;
LIST_ENTRY	NotificationList;
extern PIO_CSQ g_pCancelSafeQueue;

_Use_decl_annotations_
VOID NotificationSend(
	PNOTIFICATION_ENTRY Notification
)
{
	PIRP Irp;
	PIO_STACK_LOCATION pIrpStackLocation;
	if (g_pCancelSafeQueue != NULL)
	{
		Irp = IoCsqRemoveNextIrp(g_pCancelSafeQueue, NULL);
		if (Irp != NULL)
		{
			pIrpStackLocation = IoGetCurrentIrpStackLocation(Irp);
			if (pIrpStackLocation->Parameters.Read.Length < sizeof(OBSERVER_NOTIFICATION))
			{
				Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				Irp->IoStatus.Information = sizeof(OBSERVER_NOTIFICATION);
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
				return;
			}
			RtlCopyMemory(
				Irp->AssociatedIrp.SystemBuffer, 
				&Notification->Data, 
				sizeof(OBSERVER_NOTIFICATION)
			);
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(OBSERVER_NOTIFICATION);
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return;
		}
	}
	ExAcquireFastMutex(&NotificationListMutex);
	InsertTailList(&NotificationList, &Notification->ListEntry);
	ExReleaseFastMutex(&NotificationListMutex);
}



