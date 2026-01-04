#ifndef _tmpmof_h_
#define _tmpmof_h_

// NetKvm_Logging - NetKvm_Logging
#define NetKvm_LoggingGuid \
    { 0x234e1fbf,0x37dc,0x4882, { 0xb0,0x1e,0x18,0xf4,0x7c,0xc0,0xa4,0x0e } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(NetKvm_Logging_GUID, \
            0x234e1fbf,0x37dc,0x4882,0xb0,0x1e,0x18,0xf4,0x7c,0xc0,0xa4,0x0e);
#endif


typedef struct _NetKvm_Logging
{
    // 
    ULONG level;
    #define NetKvm_Logging_level_SIZE sizeof(ULONG)
    #define NetKvm_Logging_level_ID 1

} NetKvm_Logging, *PNetKvm_Logging;

#define NetKvm_Logging_SIZE (FIELD_OFFSET(NetKvm_Logging, level) + NetKvm_Logging_level_SIZE)

// NetKvm_Statistics - NetKvm_Statistics
#define NetKvm_StatisticsGuid \
    { 0x76ea6a8e,0xfd60,0x4f0e, { 0xb9,0x8d,0x22,0xcd,0xc8,0xb9,0x97,0x21 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(NetKvm_Statistics_GUID, \
            0x76ea6a8e,0xfd60,0x4f0e,0xb9,0x8d,0x22,0xcd,0xc8,0xb9,0x97,0x21);
#endif


typedef struct _NetKvm_Statistics
{
    // 
    ULONG rxCoalescedWin;
    #define NetKvm_Statistics_rxCoalescedWin_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_rxCoalescedWin_ID 1

    // 
    ULONG rxCoalescedHost;
    #define NetKvm_Statistics_rxCoalescedHost_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_rxCoalescedHost_ID 2

    // 
    ULONG rxChecksumOK;
    #define NetKvm_Statistics_rxChecksumOK_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_rxChecksumOK_ID 3

    // 
    ULONG rxPriority;
    #define NetKvm_Statistics_rxPriority_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_rxPriority_ID 4

    // 
    ULONG txLargeOffload;
    #define NetKvm_Statistics_txLargeOffload_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_txLargeOffload_ID 5

    // 
    ULONG txChecksumOffload;
    #define NetKvm_Statistics_txChecksumOffload_SIZE sizeof(ULONG)
    #define NetKvm_Statistics_txChecksumOffload_ID 6

} NetKvm_Statistics, *PNetKvm_Statistics;

#define NetKvm_Statistics_SIZE (FIELD_OFFSET(NetKvm_Statistics, txChecksumOffload) + NetKvm_Statistics_txChecksumOffload_SIZE)

// NetKvm_RssDiagnostics - NetKvm_RssDiagnostics
#define NetKvm_RssDiagnosticsGuid \
    { 0x7c03d07f,0x52fa,0x4c2f, { 0x8a,0x85,0x9f,0x24,0xd5,0x75,0xc5,0x18 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(NetKvm_RssDiagnostics_GUID, \
            0x7c03d07f,0x52fa,0x4c2f,0x8a,0x85,0x9f,0x24,0xd5,0x75,0xc5,0x18);
#endif


typedef struct _NetKvm_RssDiagnostics
{
    // 
    ULONG DeviceSupport;
    #define NetKvm_RssDiagnostics_DeviceSupport_SIZE sizeof(ULONG)
    #define NetKvm_RssDiagnostics_DeviceSupport_ID 1

    // 
    ULONG rxHits;
    #define NetKvm_RssDiagnostics_rxHits_SIZE sizeof(ULONG)
    #define NetKvm_RssDiagnostics_rxHits_ID 2

    // 
    ULONG rxMissed;
    #define NetKvm_RssDiagnostics_rxMissed_SIZE sizeof(ULONG)
    #define NetKvm_RssDiagnostics_rxMissed_ID 3

    // 
    ULONG rxUnclassified;
    #define NetKvm_RssDiagnostics_rxUnclassified_SIZE sizeof(ULONG)
    #define NetKvm_RssDiagnostics_rxUnclassified_ID 4

    // 
    ULONG rxErrors;
    #define NetKvm_RssDiagnostics_rxErrors_SIZE sizeof(ULONG)
    #define NetKvm_RssDiagnostics_rxErrors_ID 5

} NetKvm_RssDiagnostics, *PNetKvm_RssDiagnostics;

#define NetKvm_RssDiagnostics_SIZE (FIELD_OFFSET(NetKvm_RssDiagnostics, rxErrors) + NetKvm_RssDiagnostics_rxErrors_SIZE)

// NetKvm_Standby - NetKvm_Standby
#define NetKvm_StandbyGuid \
    { 0x15c62b55,0xf41f,0x4784, { 0xae,0xce,0xe9,0x95,0x2b,0xa5,0x95,0x1b } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(NetKvm_Standby_GUID, \
            0x15c62b55,0xf41f,0x4784,0xae,0xce,0xe9,0x95,0x2b,0xa5,0x95,0x1b);
#endif


typedef struct _NetKvm_Standby
{
    // 
    ULONG value;
    #define NetKvm_Standby_value_SIZE sizeof(ULONG)
    #define NetKvm_Standby_value_ID 1

} NetKvm_Standby, *PNetKvm_Standby;

#define NetKvm_Standby_SIZE (FIELD_OFFSET(NetKvm_Standby, value) + NetKvm_Standby_value_SIZE)

#endif
