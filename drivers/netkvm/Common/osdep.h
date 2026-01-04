/*
 * This file contains highest level definitions, mainly for non-NDIS modules,
 * when they are involved into NDIS driver
 *
 * Copyright (c) 2008-2017 Red Hat, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met :
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and / or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of their contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#pragma once

#define _NO_CRT_STDIO_INLINE
#include <ndis.h>

#ifndef _WIN32_WCE
#include <Ntstrsafe.h>
#else
#include <windows.h>
#include <ceddk.h>
#include <devload.h>
#include <cesvsbus.h>
#include <pcibus.h>

#ifndef KVMNETDRIVER
typedef struct _PCI_CAPABILITIES_HEADER {
	UCHAR CapabilityID;
	UCHAR Next;
} PCI_CAPABILITIES_HEADER, *PPCI_CAPABILITIES_HEADER;
#endif

#ifndef __drv_interlocked
#define __drv_interlocked
#endif

#ifndef PCI_WHICHSPACE_CONFIG
#define PCI_WHICHSPACE_CONFIG             0x0
#endif

#ifndef CM_RESOURCE_INTERRUPT_MESSAGE
#define CM_RESOURCE_INTERRUPT_MESSAGE         0x0002
#endif

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
#endif

#ifndef RtlUshortByteSwap
#define RtlUshortByteSwap(X) (((X & 0xFF00) >> 8) | ((X & 0xFF) << 8))
#endif

#if NTDDI_VERSION <= NTDDI_VISTASP1
#define _Requires_lock_held_(lock)
#define _Acquires_shared_lock_(lock)
#define _Acquires_exclusive_lock_(lock)
#endif

#if ((OSVERSION_MASK & NTDDI_VERSION) == NTDDI_VISTA)
#define NDIS_PROTOCOL_ID_IP6            0x03

#ifndef offsetof
#ifdef  _WIN64
#define offsetof(s,f)   ((size_t)((ptrdiff_t)&(((s *)0)->f)))
#else
#define offsetof(s,f)   ((size_t)&(((s *)0)->f))
#endif
#endif

#ifndef KVMNETDRIVER
typedef struct _NETWORK_ADDRESS_IP6 {
    USHORT      sin6_port;
    ULONG       sin6_flowinfo;
    USHORT      sin6_addr[8];
    ULONG       sin6_scope_id;
} NETWORK_ADDRESS_IP6, *PNETWORK_ADDRESS_IP6;
#endif
#endif

#ifndef PARANDIS_MAJOR_DRIVER_VERSION
#define PARANDIS_MAJOR_DRIVER_VERSION 6
#endif

#ifndef PARANDIS_MINOR_DRIVER_VERSION
#define PARANDIS_MINOR_DRIVER_VERSION 1
#endif

#if (7-PARANDIS_MAJOR_DRIVER_VERSION-7) == 14
#undef PARANDIS_MAJOR_DRIVER_VERSION
#define PARANDIS_MAJOR_DRIVER_VERSION       101
#endif

#if (7-PARANDIS_MINOR_DRIVER_VERSION-7) == 14
#undef PARANDIS_MINOR_DRIVER_VERSION
#define PARANDIS_MINOR_DRIVER_VERSION       58000
#endif

#ifndef _WIN32_WCE
#ifndef KI_USER_SHARED_DATA
#ifdef _X86_
#define KI_USER_SHARED_DATA 0xFFDF0000
#elif defined(_ARM_)
#define KI_USER_SHARED_DATA 0xFFFF9000
#else
#define KI_USER_SHARED_DATA 0xFFFFF78000000000UI64
#endif
#endif

#ifndef SharedSystemTime
#define SharedSystemTime (KI_USER_SHARED_DATA + 0x14)
#endif

#ifndef KeQuerySystemTime
#define KeQuerySystemTime(CurrentCount)                                     \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedSystemTime))
#endif
#endif
