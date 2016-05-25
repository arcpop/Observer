#ifndef NOTIFICATION_DATA_H
#define NOTIFICATION_DATA_H
#pragma once

#ifdef NTDDI_VERSION
#include <ntddk.h>
#else

#endif 



typedef struct _NOTIFICATION_DATA {
	UINT32	NotificationType;
	UINT32	Reaction;
	UINT64	CurrentProcessID;
	UINT64	CurrentThreadID;
	union
	{
		struct
		{
			UINT16	RegistryAction;
			WCHAR	RegistryPath[250];
		} RegistryAction;

		struct
		{
			UINT64	TargetProcessID;
			UINT32	OpenFlags;
		} ProcessOpened;

		struct
		{
			UINT64	TargetThreadID;
			UINT32	OpenFlags;
		} ThreadOpened;

		struct
		{
			WCHAR	ImageNamePath[1000];
		} ProcessCreated;

		struct
		{
			UINT64 RemoteProcessID;
		} RemoteThreadCreated;

		struct
		{
			UINT64	ImageBase;
			UINT64	ImageSize;
			WCHAR	ImageName[250];
		} ImageMapped;
	};
} NOTIFICATION_DATA, *PNOTIFICATION_DATA;

#endif // !NOTIFICATION_DATA_H
