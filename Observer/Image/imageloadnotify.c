#include "ImageLoadNotify.h"

VOID HandleDriverLoad(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ PIMAGE_INFO ImageInfo
)
{
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ImageInfo);
}

VOID HandleExecutableImageLoad(
	_In_ PEPROCESS Process,
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ PIMAGE_INFO ImageInfo
)
{
	UNREFERENCED_PARAMETER(Process);
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ImageInfo);
}
VOID ObserverImageLoadNotify(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,
	_In_ PIMAGE_INFO ImageInfo
)
{
	if (ProcessId == NULL)
	{
		HandleDriverLoad(FullImageName, ImageInfo);
		return;
	}
	else
	{
		PEPROCESS Process = NULL;
		NTSTATUS Status;
		Status = ObReferenceObjectByHandle(ProcessId, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &Process, NULL);
		if (!NT_SUCCESS(Status))
		{
			HandleExecutableImageLoad(Process, FullImageName, ImageInfo);
			return;
		}
		HandleExecutableImageLoad(Process, FullImageName, ImageInfo);
		ObDereferenceObject(Process);
		return;
	}
}
