#ifndef PROCESS_H
#define PROCESS_H
#pragma once

#include <ntddk.h>
#include "Rule.h"

NTSTATUS ProcessObserverInitialize();
NTSTATUS ProcessObserverUnload();

NTSTATUS ProcessObserverAddRule(
	_In_ POBSERVER_PROCESS_CREATION_RULE Rule,
	_In_ POBSERVER_RULE_HANDLE			 RuleHandle
);

NTSTATUS ProcessObserverRemoveRule(
	_In_ POBSERVER_RULE_HANDLE RuleHandle
);

#endif // !PROCESS_H