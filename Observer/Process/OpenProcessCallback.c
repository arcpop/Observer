#include "ProcessObserver.h"
#include "../Log/Log.h"

#ifndef PROCESS_VM_WRITE
#define PROCESS_VM_WRITE       (0x0020)
#endif //PROCESS_VM_WRITE

#ifndef PROCESS_VM_OPERATION
#define PROCESS_VM_OPERATION   (0x0008)
#endif //PROCESS_VM_OPERATION

OB_CALLBACK_REGISTRATION OpenProcessCallbacks = { 0 };
OB_OPERATION_REGISTRATION OpenProcessCallbacksOperations[2] = { { 0 }, { 0 } };
UNICODE_STRING OpenProcessCallbacksAltitude;
PVOID OpenProcessCallbacksHandle = NULL;

VOID ReportProcessOpen(
	_In_ PEPROCESS ProcessToOpen, 
	_In_ ACCESS_MASK Access
)
{
	/*HANDLE ProcessIdToOpen, CurrentProcessId;
	
	ProcessIdToOpen = PsGetProcessId(ProcessToOpen);
	CurrentProcessId = PsGetCurrentProcessId();

	DEBUG_LOG("Open:  Current %d, Target %d, Access 0x%.8X", CurrentProcessId, ProcessIdToOpen, Access);
	*/
	UNREFERENCED_PARAMETER(ProcessToOpen);
	UNREFERENCED_PARAMETER(Access);
}

OB_PREOP_CALLBACK_STATUS PreOpenProcess(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
)
{
	if (PreInfo->KernelHandle == TRUE)
	{
		return OB_PREOP_SUCCESS;
	}
	if (PreInfo->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		if (PreInfo->Object == PsGetCurrentProcess())
		{
			return OB_PREOP_SUCCESS;
		}
		if (PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess & (PROCESS_VM_OPERATION | PROCESS_VM_WRITE))
		{
			ReportProcessOpen(PreInfo->Object, PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess);
			//We do not allow memory changing operations on other processes
			//PreInfo->Parameters->CreateHandleInformation.DesiredAccess &= ~(PROCESS_VM_OPERATION | PROCESS_VM_WRITE);
		}
	} 
	else if (PreInfo->Operation == OB_OPERATION_HANDLE_DUPLICATE)
	{
		PEPROCESS CurrentProcess = PsGetCurrentProcess();
		//Allow duplication with any access if the object is the current process and the handle ends up in the current process
		if (PreInfo->Object == CurrentProcess &&
			PreInfo->Parameters->DuplicateHandleInformation.TargetProcess == CurrentProcess)
		{
			return OB_PREOP_SUCCESS;
		}
		if (PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & (PROCESS_VM_OPERATION | PROCESS_VM_WRITE))
		{
			ReportProcessOpen(PreInfo->Object, PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess);
			//We do not allow memory changing operations on other processes
			//PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess &= ~(PROCESS_VM_OPERATION | PROCESS_VM_WRITE);
		}
	}

	UNREFERENCED_PARAMETER(RegistrationContext);
	return OB_PREOP_SUCCESS;
}

VOID PostOpenProcess(
	_In_ PVOID RegistrationContext,
	_In_ POB_POST_OPERATION_INFORMATION PostInfo
)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(PostInfo);
}

OB_PREOP_CALLBACK_STATUS PreOpenThread(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(PreInfo);
	return OB_PREOP_SUCCESS;
}

VOID PostOpenThread(
	_In_ PVOID RegistrationContext,
	_In_ POB_POST_OPERATION_INFORMATION PostInfo
)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(PostInfo);
}

NTSTATUS OpenProcessCallbacksInitialize()
{
	NTSTATUS Status;


	OpenProcessCallbacksOperations[0].ObjectType = PsProcessType;
	OpenProcessCallbacksOperations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
	OpenProcessCallbacksOperations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
	OpenProcessCallbacksOperations[0].PreOperation = PreOpenProcess;
	OpenProcessCallbacksOperations[0].PostOperation = PostOpenProcess;

	OpenProcessCallbacksOperations[1].ObjectType = PsThreadType;
	OpenProcessCallbacksOperations[1].Operations |= OB_OPERATION_HANDLE_CREATE;
	OpenProcessCallbacksOperations[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
	OpenProcessCallbacksOperations[1].PreOperation = PreOpenThread;
	OpenProcessCallbacksOperations[1].PostOperation = PostOpenThread;

	RtlInitUnicodeString(&OpenProcessCallbacksAltitude, L"1000");

	OpenProcessCallbacks.Version = OB_FLT_REGISTRATION_VERSION;
	OpenProcessCallbacks.OperationRegistrationCount = 2;
	OpenProcessCallbacks.Altitude = OpenProcessCallbacksAltitude;
	OpenProcessCallbacks.RegistrationContext = NULL;
	OpenProcessCallbacks.OperationRegistration = OpenProcessCallbacksOperations;

	Status = ObRegisterCallbacks(
		&OpenProcessCallbacks, 
		&OpenProcessCallbacksHandle
	);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}
	return STATUS_SUCCESS;
}

VOID OpenProcessCallbacksUnload()
{
	if (OpenProcessCallbacksHandle != NULL)
	{
		ObUnRegisterCallbacks(OpenProcessCallbacksHandle);
	}
}