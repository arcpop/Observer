#ifndef NOTIFICATION_H
#define NOTIFICATION_H
#pragma once

#ifdef NTDDI_VERSION
#include <ntddk.h>
#else

#endif 

#define NOTIFICATION_STRING_BUFFER_SIZE 1000

#define NOTIFICATION_REGISTRY_ACTION_SET_VALUE		1
#define NOTIFICATION_REGISTRY_ACTION_CREATE_KEY		2


typedef struct _OBSERVER_NOTIFICATION {
	UINT32	NotificationType;
	UINT32	Reaction;
	UINT64	CurrentProcessID;
	UINT64	CurrentThreadID;
	union
	{
		struct
		{
			UINT16	RegistryAction;
			WCHAR	RegistryPath[NOTIFICATION_STRING_BUFFER_SIZE];
		} Registry;
		struct
		{
			ULONG64 NewThreadID;
			ULONG64 TargetProcessID;
		} ThreadCreated;
		struct
		{
			ULONG64 NewProcessID;
			ULONG64 ParentProcessID;
			WCHAR	ImageNamePath[NOTIFICATION_STRING_BUFFER_SIZE];
		} ProcessCreated;
		struct
		{
			UINT64	ImageBase;
			UINT64	ImageSize;
			UINT32  ImageSigned		: 1;
			UINT32  SystemImage		: 1;
			UINT32	Reserved		: 30;
			WCHAR	ImageName[NOTIFICATION_STRING_BUFFER_SIZE];
		} ModuleLoaded;
		struct
		{
			UINT64	ImageBase;
			UINT64	ImageSize;
			UINT32  ImageSigned : 1;
			UINT32  SystemImage : 1;
			UINT32	Reserved : 30;
			WCHAR	ImageName[NOTIFICATION_STRING_BUFFER_SIZE];
		} DriverLoaded;
	} Types;
} OBSERVER_NOTIFICATION, *POBSERVER_NOTIFICATION;

#endif // !NOTIFICATION_H
