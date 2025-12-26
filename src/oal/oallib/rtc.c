/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  rtc.c
//
//  This file implements OAL real time module. 
//
//  Implementation uses system tick to calculate realtime clock. Tritons
//  chip driver should call IOCTL_HAL_INIT_RTC periodically to update
//  internal RTC time with real one.
//
//
#include <windows.h>

#if (_WINCEOSVER>=600)
#include <nkexport.h>
#endif

#include <windows.h>
#include <oemglobal.h>
#include <nkexport.h>
#include <ddhal.h>

#include "pl031-rtc.h"
#include "mmio.h"

#define mmio_write32(reg, data)  *(volatile DWORD*)(reg) = (data)
#define mmio_read32(reg)  *(volatile DWORD*)(reg)

//------------------------------------------------------------------------------
//
//  Define:  RTC_BASE_YEAR
//
//  Delta from which RTC counts years
//  Resolution of RTC years is from 2000 to 2099
//
#define RTC_BASE_YEAR_MIN       2000
#define RTC_BASE_YEAR_MAX       2099


//------------------------------------------------------------------------------

#define BCD2BIN(b)              (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)              ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

VOID OALLogPrintf(LPWSTR buffer, UINT32 maxChars, LPCWSTR format, ...)
{
	va_list pArgList;

	va_start(pArgList, format);
	NKwvsprintfW(buffer, format, pArgList, maxChars);
}

//------------------------------------------------------------------------------

static struct {

    BOOL       initialized;     // Is RTC subsystem intialized?

    CRITICAL_SECTION cs;

    pl031_local pl031;          // pl031 driver

    ULONGLONG  baseFiletime;    // Base filetime
    DWORD      baseTickCount;   // Tick count from base filetime

    ULONGLONG  alarmFiletime;   // Alarm filetime 

} s_rtc = { FALSE };


//------------------------------------------------------------------------------

UINT32
OEMGetTickCount(
                );

BOOL
FiletimeToHWTime(
                 ULONGLONG fileTime, 
                 UCHAR bcdTime[6]
);

//------------------------------------------------------------------------------

LPCWSTR
SystemTimeToString(
                   SYSTEMTIME *pSystemTime
                   )
{
    static WCHAR buffer[64];

    OALLogPrintf(
        buffer, 64, L"%04d.%02d.%02d %02d:%02d:%02d.%03d",
        pSystemTime->wYear, pSystemTime->wMonth, pSystemTime->wDay,
        pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond, 
        pSystemTime->wMilliseconds
        );        
    return buffer;
}

//------------------------------------------------------------------------------

LPCWSTR
HWTimeToString(
               UCHAR bcdTime[6]
)
{
    static WCHAR buffer[64];

    OALLogPrintf(
        buffer, 64, L"%04d.%02d.%02d %02d:%02d:%02d",
        BCD2BIN(bcdTime[5]) + RTC_BASE_YEAR_MIN, 
        BCD2BIN(bcdTime[4]), 
        BCD2BIN(bcdTime[3]),
        BCD2BIN(bcdTime[2]), 
        BCD2BIN(bcdTime[1]), 
        BCD2BIN(bcdTime[0])
        );        
    return buffer;
}

//------------------------------------------------------------------------------

VOID
ReadBaseTime(
               ULONGLONG   *pOffset
               )
{
    DWORD val = 0;

    if (pOffset == NULL)
    {
        return;
    }

    // Read backup registers for secure time offset
    *pOffset = 0;

    val = mmio_read32((ULONGLONG)s_rtc.pl031.base + RTC_DR);

    *pOffset = val;
}

//------------------------------------------------------------------------------

VOID
WriteBaseTime(
                ULONGLONG   *pOffset
                )
{
    DWORD val = 0;

    if (pOffset == NULL)
    {
        return;
    }

    // Write backup registers with secure time offset
    val = (DWORD)*pOffset;

	mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_LR, val);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcTime
//
//  This function is called by RTC driver when time event interrupt
//  occurs.
//
BOOL
OALIoCtlHalRtcTime(
UINT32 code,
VOID *pInBuffer,
UINT32 inSize,
VOID *pOutBuffer,
UINT32 outSize,
UINT32 *pOutSize
)
{
	DEBUGMSG(1, L"+OALIoCtlHalRtcTime()\r\n");

	if (!s_rtc.initialized)
	{
		s_rtc.pl031.base = (void *)RTC_BASE;
		s_rtc.initialized = TRUE;
		s_rtc.baseTickCount = OEMGetTickCount();
	}

	EnterCriticalSection(&s_rtc.cs);

	// Read base time
	ReadBaseTime(&s_rtc.baseFiletime);

	//  Reset the tick count
	s_rtc.baseTickCount = OEMGetTickCount();

	LeaveCriticalSection(&s_rtc.cs);

	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot. 
//  Input buffer contains SYSTEMTIME structure with default time value.
//
//
BOOL
OALIoCtlHalInitRTC(
                   UINT32 code, 
                   VOID *pInBuffer, 
                   UINT32 inSize, 
                   VOID *pOutBuffer, 
                   UINT32 outSize, 
                   UINT32 *pOutSize
                   )
{
    BOOL            rc = FALSE;

    SYSTEMTIME      *pGivenTime = (LPSYSTEMTIME) pInBuffer;
    UCHAR           bcdTime[6];
    UCHAR           status = 1;
    UCHAR           secure = 0;
    DWORD           dwRegVal = 0;
    ULONG           time = 0;

	DEBUGMSG(1, L"+OALIoCtlHalInitRTC()\r\n");

    // Initialize RTC critical section
    InitializeCriticalSection(&s_rtc.cs);

    // First read RTC status from Triton
	s_rtc.pl031.base = (void *)RTC_BASE;

    /*
     * On ST PL031 variants, the RTC reset value does not provide correct
     * weekday for 2000-01-01. Correct the erroneous sunday to saturday.
     */
    if (mmio_read32((ULONGLONG)s_rtc.pl031.base + RTC_YDR) == 0x2000) {
        time = mmio_read32((ULONGLONG)s_rtc.pl031.base + RTC_DR);
        if ((time &
            (RTC_MON_MASK | RTC_MDAY_MASK | RTC_WDAY_MASK))
            == 0x02120000) {
            time = time | (0x7 << RTC_WDAY_SHIFT);
            mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_YLR, 0x2000);
			mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_LR, time);
        }
    }

    // Check for a clean boot of device - if so, reset date/time to system default (LTK2026)
	DEBUGMSG(1, L"OALIoCtlHalInitRTC(): Getting RTC\r\n");

    //  Read the base time
    ReadBaseTime( &s_rtc.baseFiletime );

	// Convert time
	FiletimeToHWTime(s_rtc.baseFiletime, bcdTime);

	DEBUGMSG(1, L"OALIoCtlHalInitRTC(): RTC = ");
	DEBUGMSG(1, HWTimeToString(bcdTime));
	DEBUGMSG(1, L"\r\n");

    // Now update RTC state values
	s_rtc.initialized = TRUE;
	s_rtc.baseTickCount = OEMGetTickCount();

    //  Success
    rc = TRUE;

	DEBUGMSG(1, L"-OALIoCtlHalInitRTC() end\r\n");

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
BOOL
OEMGetRealTime(
               SYSTEMTIME *pSystemTime
               ) 
{
    DWORD       delta;
    ULONGLONG   time;

	DEBUGMSG(1, L"+OEMGetRealTime()\r\n");

    if (!s_rtc.initialized)
    {
		s_rtc.pl031.base = (void *)RTC_BASE;
		s_rtc.initialized = TRUE;
		s_rtc.baseTickCount = OEMGetTickCount();
	}

	EnterCriticalSection(&s_rtc.cs);
    delta = OEMGetTickCount() - s_rtc.baseTickCount;
	ReadBaseTime(&s_rtc.baseFiletime);
    time = s_rtc.baseFiletime + ((ULONGLONG)delta) * 10000;
    NKFileTimeToSystemTime((FILETIME*)&time, pSystemTime);
    LeaveCriticalSection(&s_rtc.cs);

	DEBUGMSG(1, L"-OEMGetRealTime() = ");
	DEBUGMSG(1, SystemTimeToString(pSystemTime));
	DEBUGMSG(1, L"\r\n");

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
BOOL
OEMSetRealTime(
               SYSTEMTIME *pSystemTime
               ) 
{
    BOOL        rc = FALSE;

    ULONGLONG   fileTime;
    DWORD       tickDelta;

	DEBUGMSG(1, L"+OEMSetRealTime(");
	DEBUGMSG(1, SystemTimeToString(pSystemTime));
	DEBUGMSG(1, L"\r\n");

	if (!s_rtc.initialized)
	{
		s_rtc.pl031.base = (void *)RTC_BASE;
		s_rtc.initialized = TRUE;
		s_rtc.baseTickCount = OEMGetTickCount();
	}

    // Save time to global structure
    EnterCriticalSection(&s_rtc.cs);

    // Round to seconds
    pSystemTime->wMilliseconds = 0;

    // Convert to filetime
    if (NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&fileTime))
    {
        // Compute the tick delta (indicates the time in the RTC)
        tickDelta = OEMGetTickCount() - s_rtc.baseTickCount;

        // Update all the parameters
        s_rtc.baseFiletime  = s_rtc.baseFiletime + ((ULONGLONG)tickDelta)*10000;
        s_rtc.baseTickCount = OEMGetTickCount();

        //  Save off base offset to the backup regs
        WriteBaseTime( &s_rtc.baseFiletime );

        // Done
        rc = TRUE;

        LeaveCriticalSection(&s_rtc.cs);
    }

	DEBUGMSG(1, L"-OEMSetRealTime\r\n");

    return rc;
}

static void pl031_alarm_irq_enable(BOOL enabled)
{
    ULONG imsc = 0;

	if (!s_rtc.initialized)
	{
		s_rtc.pl031.base = (void *)RTC_BASE;
		s_rtc.initialized = TRUE;
		s_rtc.baseTickCount = OEMGetTickCount();
	}

    /* Clear any pending alarm interrupts. */
	mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_ICR, RTC_BIT_AI);

    imsc = mmio_read32((ULONGLONG)s_rtc.pl031.base + RTC_IMSC);

    if (enabled == 1)
		mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_IMSC, imsc | RTC_BIT_AI);
    else
		mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_IMSC, imsc & ~RTC_BIT_AI);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  This function is called by the kernel to set the real-time clock alarm.
//
BOOL
OEMSetAlarmTime(
                SYSTEMTIME *pSystemTime
                ) 
{
    BOOL rc = FALSE;

	DEBUGMSG(1, L"+OEMSetAlarmTime(");
	DEBUGMSG(1, SystemTimeToString(pSystemTime));
	DEBUGMSG(1, L"\r\n");

	if (!s_rtc.initialized)
	{
		s_rtc.pl031.base = (void *)RTC_BASE;
		s_rtc.initialized = TRUE;
		s_rtc.baseTickCount = OEMGetTickCount();
	}

    // Save time to global structure
    EnterCriticalSection(&s_rtc.cs);

    // Time Sync with kernel time
	OALIoCtlHalRtcTime(0, NULL, 0, NULL, 0, NULL);
    // Round to seconds
    pSystemTime->wMilliseconds = 0;

    // Convert to filetime
    if (NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&s_rtc.alarmFiletime))
    {
        UCHAR   bcdTime[6];
        SYSTEMTIME CurSystemTime;
        ULONG FileTime, AlarmTime;

        //  Convert to BCD time format
        FiletimeToHWTime( s_rtc.alarmFiletime, bcdTime );

        //  Check if alarm time is valid
        OEMGetRealTime(&CurSystemTime);
        NKSystemTimeToFileTime( &CurSystemTime, (FILETIME*)&FileTime);
        NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&AlarmTime);

        //  Convert time in secs, ignore miliseconds
        AlarmTime = (ULONGLONG) AlarmTime/10000000;
        FileTime = (ULONGLONG) FileTime/10000000;

        //  Verify if alarm is set in the past or current
        if (AlarmTime <= FileTime)
        {
            // Alarm is in the past, not valid alarm
            rc = FALSE;
            LeaveCriticalSection(&s_rtc.cs);
            return rc;
        }

        mmio_write32((ULONGLONG)s_rtc.pl031.base + RTC_MR, AlarmTime);
		pl031_alarm_irq_enable(TRUE);

		DEBUGMSG(1, L"+OEMSetAlarmTime Alarm set. resetting\r\n");

        // Done
        rc = TRUE;
    }

    LeaveCriticalSection(&s_rtc.cs);

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcAlarm
//
//  This function is called by RTC driver when alarm interrupt
//  occurs.
//
BOOL
OALIoCtlHalRtcAlarm(
                    UINT32 code, 
                    VOID *pInBuffer, 
                    UINT32 inSize, 
                    VOID *pOutBuffer, 
                    UINT32 outSize, 
                    UINT32 *pOutSize
                    )
{
	DEBUGMSG(1, L"+OALIoCtlHalRtcAlarm()\r\n");

    //Time Sync with kernel time
    OALIoCtlHalRtcTime(0, NULL, 0, NULL, 0, NULL);

    //  Alarm has been triggered by RTC driver.
	pl031_alarm_irq_enable(FALSE);

	return TRUE;
}

//------------------------------------------------------------------------------

BOOL
FiletimeToHWTime(
                 ULONGLONG fileTime, 
                 UCHAR bcdTime[6]
)
{
    SYSTEMTIME systemTime;

    //  Convert filetime to RTC HW time format
    NKFileTimeToSystemTime((FILETIME*)&fileTime, &systemTime);

    //  Limit RTC year range
    if( systemTime.wYear < RTC_BASE_YEAR_MIN )
        systemTime.wYear = RTC_BASE_YEAR_MIN;

    if( systemTime.wYear > RTC_BASE_YEAR_MAX )
        systemTime.wYear = RTC_BASE_YEAR_MAX;

    bcdTime[5] = BIN2BCD(systemTime.wYear - RTC_BASE_YEAR_MIN);
    bcdTime[4] = BIN2BCD(systemTime.wMonth);
    bcdTime[3] = BIN2BCD(systemTime.wDay);
    bcdTime[2] = BIN2BCD(systemTime.wHour);
    bcdTime[1] = BIN2BCD(systemTime.wMinute);
    bcdTime[0] = BIN2BCD(systemTime.wSecond);

    return TRUE;
}

//------------------------------------------------------------------------------

