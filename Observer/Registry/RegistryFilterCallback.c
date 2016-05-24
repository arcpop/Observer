#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterCallback(
	PVOID CallbackContext,
	PVOID Argument1,
	PVOID Argument2
)
{
	REG_NOTIFY_CLASS NotifyClass;
	PREGISTRY_FILTER_CONTEXT pContext;

	if (Argument2 == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pContext = (PREGISTRY_FILTER_CONTEXT)CallbackContext;
	NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

	switch (NotifyClass)
	{
	case RegNtPostCreateKey:
		return RegistryFilterPostCreateKey(pContext, (PREG_POST_CREATE_KEY_INFORMATION)Argument2);
	case RegNtPostCreateKeyEx:
		return RegistryFilterPostCreateKeyEx(pContext, (PREG_POST_OPERATION_INFORMATION)Argument2);

	case RegNtPostOpenKey:
		return RegistryFilterPostOpenKey(pContext, (PREG_POST_OPEN_KEY_INFORMATION)Argument2);
	case RegNtPostOpenKeyEx:
		return RegistryFilterPostOpenKeyEx(pContext, (PREG_POST_OPERATION_INFORMATION)Argument2);

	case RegNtPreSetValueKey:
		return RegistryFilterPreSetValueKey(pContext, (PREG_SET_VALUE_KEY_INFORMATION)Argument2);

	case RegNtCallbackObjectContextCleanup:
		return RegistryFilterCleanupObjectContext(pContext, (PREG_CALLBACK_CONTEXT_CLEANUP_INFORMATION)Argument2);
	}
	return STATUS_SUCCESS;
}
