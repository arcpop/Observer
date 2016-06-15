#ifndef UTIL_H
#define UTIL_H
#pragma once

#include <ntddk.h>

BOOLEAN UtilUnicodeStringContains(
	_In_ PCUNICODE_STRING	String,
	_In_ PCUNICODE_STRING	SubString,
	_In_ BOOLEAN			IgnoreCase
);


#endif // !UTIL_H
