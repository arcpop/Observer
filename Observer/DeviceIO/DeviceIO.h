#ifndef DEVICE_IO_H
#define DEVICE_IO_H

#pragma once

#include <ntddk.h>

const wchar_t DeviceName[]		= L"\\Device\\Observer";
const wchar_t DosDeviceName[]	= L"\\DosDevices\\Observer";

NTSTATUS  DeviceIOInitialize(
	_In_ PDRIVER_OBJECT DriverObject
);

NTSTATUS  DeviceIOUnload(
	_In_ PDRIVER_OBJECT DriverObject
);

DRIVER_DISPATCH  DeviceIOCreate;
DRIVER_DISPATCH  DeviceIORead;
DRIVER_DISPATCH  DeviceIOControl;
DRIVER_DISPATCH  DeviceIOCleanup;
DRIVER_DISPATCH  DeviceIOClose;

#endif // !DEVICE_IO_H
