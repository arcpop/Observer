#ifndef REGISTRY_FILTER_H
#define REGISTRY_FILTER_H

#include <ntddk.h>

#include "Rule.h"

NTSTATUS RegistryFilterInitialize(
	_In_     PDRIVER_OBJECT pDriverObject,
	_Out_	 PVOID* ppContext
);

NTSTATUS RegistryFilterUnload(
	_In_     PVOID pContext
);

NTSTATUS RegistryFilterAddRule(
	_In_	POBSERVER_REGISTRY_RULE Rule,
	_Out_	POBSERVER_RULE_HANDLE RuleHandle
);

NTSTATUS RegistryFilterRemoveRule(
	_In_	POBSERVER_RULE_HANDLE RuleHandle
);

#endif // !REGISTRY_FILTER_H