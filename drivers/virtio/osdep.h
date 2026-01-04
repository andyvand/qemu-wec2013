//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007  Qumranet All Rights Reserved
//
// Module Name:
// osdep.h
//
// Abstract:
// Windows OS dependent definitions of data types
//
// Author:
// Yan Vugenfirer  - February 2007.
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32_WCE
#include <windows.h>
#include <ceddk.h>
#include <devload.h>
#include <cesvsbus.h>
#include <pcibus.h>

typedef struct _PCI_CAPABILITIES_HEADER {
	UCHAR CapabilityID;
	UCHAR Next;
} PCI_CAPABILITIES_HEADER, *PPCI_CAPABILITIES_HEADER;

#ifndef PPCI_COMMON_HEADER
#define PPCI_COMMON_HEADER PPCI_COMMON_CONFIG
#endif

#ifndef PCI_COMMON_HEADER
#define PCI_COMMON_HEADER PCI_COMMON_CONFIG
#endif

#ifndef PCI_CARDBUS_BRIDGE_TYPE
#define PCI_CARDBUS_BRIDGE_TYPE           0x02
#endif

#ifndef PCI_STATUS_CAPABILITIES_LIST
#define PCI_STATUS_CAPABILITIES_LIST      0x0010
#endif

#ifndef PCI_CAPABILITY_ID_VENDOR_SPECIFIC
#define PCI_CAPABILITY_ID_VENDOR_SPECIFIC   0x09
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(X) (sizeof(X) / sizeof(X[0]))
#endif
#else
#include <ntddk.h>
#endif

#if !defined(ENOSPC)
#define ENOSPC 1
#endif

#if !defined(__cplusplus) && !defined(bool)
// Important note: in MSFT C++ bool length is 1 bytes
// C++ does not define length of bool
// inconsistent definition of 'bool' may create compatibility problems
#define bool u8
#define false FALSE
#define true TRUE
#endif

#define inline __forceinline
#define SMP_CACHE_BYTES 64
