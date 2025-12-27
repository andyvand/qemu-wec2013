/*****************************************************************************
   
    Copyright (c) 2004-2011 SMSC. All rights reserved.

    Use of this source code is subject to the terms of the SMSC Software
    License Agreement (SLA) under which you licensed this software product.  
    If you did not accept the terms of the SLA, you are not authorized to use
    this source code. 

    This code and information is provided as is without warranty of any kind,
    either expressed or implied, including but not limited to the implied
    warranties of merchantability and/or fitness for a particular purpose.
     
    
*****************************************************************************/
/*lint -save*/
/*lint -e14 -e43 -e46 -e123 -e427 -e537 -e620 -e652 -e659 -e683*/
/*lint -e726 -e760 -e761 -e762 -e763 -e767 -e773 -e793 -e806 -e828 -e912*/
/*lint -e935 -e937 -e950 -e955 -e956 -e957 -e958 -e959 -e960 -e961 -e962*/
/*lint -e973 -e1916*/
#include <windows.h>
#include <Pkfuncs.h>
/*lint -restore*/
#include "smsc9118.h"

#define BULVERDE_BASE_REG_PA_MEMC 0x48000000

typedef struct 
{
    UINT32   ddadr;    // descriptor address reg.
    UINT32   dsadr;    // source address register.
    UINT32   dtadr;    // target address register.
    UINT32   dcmd;     // command address register.

} DMADescriptorChannelType;

typedef struct
{
	UINT32    dcsr[32];	     // DMA CSRs by channel.
	UINT32    rsvd0[0x8];
	UINT32    dalign;
	UINT32    dpcsr;
	UINT32    rsvd1[0xE];
	UINT32    drqsr0;        // Dreq[0] Status.
	UINT32    drqsr1;        // Dreq[1] Status.
	UINT32    drqsr2;        // Dreq[1] Status.
	UINT32    rsvd2[0x1];
	UINT32    dint;          // DMA interrupt Register.
	UINT32    rsvd3[0x3];
	UINT32    drcmr[64];     // On-chip device DMA request --> channel map registers [63:0].
	DMADescriptorChannelType ddg[32];    // 32 channels of descriptor registers.
	UINT32    rsvd4[0x340];
	UINT32    drcmr2[11];    // On-chip device DMA request --> channel map registers [63:0].
} BULVERDE_DMA_REG, *PBULVERDE_DMA_REG;

static volatile PDWORD      pdwGpioCtrl;

extern DWORD    ReadCLKCFG(void);

void PlatformSetBusWidth(const DWORD dwBusWidth)
{
    PHYSICAL_ADDRESS    PhysAddr;
    volatile PDWORD     pdwMemCtrl;

    PhysAddr.LowPart = (DWORD)BULVERDE_BASE_REG_PA_MEMC;
    PhysAddr.HighPart = -1L;
    pdwMemCtrl = MmMapIoSpace(PhysAddr, (ULONG)PAGE_SIZE, (BOOLEAN)FALSE);
    if (pdwMemCtrl == NULL)
    {
        SMSC_WARNING0("Failed to get Virtual address of MemController\r\n");
    }
    else
    {
        // pdwMemCtrl[4]  : MSC2
        if (dwBusWidth == 16UL)
        {
            pdwMemCtrl[4] = pdwMemCtrl[4] | 0x00080000UL;
        }
        else if (dwBusWidth == 32UL)
        {
            pdwMemCtrl[4] = pdwMemCtrl[4] & 0xFFF7FFFFUL;
        }
        else
        {
            SMSC_WARNING0("Wrong Bus Width. Should be 16 ro 32. Set to 32bit as default.\r\n");
            pdwMemCtrl[4] = pdwMemCtrl[4] & 0xFFF7FFFFUL;
        }
    }

    MmUnmapIoSpace(pdwMemCtrl, (ULONG)PAGE_SIZE);
}

/* PlatformInitialize: 
 *   perform any platform initialization necessary to make the Lan9118 visible.
 *   This function must be called before PlatformGetLanBase
 */
void PlatformInitialize()
{
    PHYSICAL_ADDRESS    PhysAddr;
    volatile PDWORD     pdwMemCtrl;

    SMSC_TRACE0(DBG_INIT, "+PlatformInitialize()\r\n");

    PhysAddr.LowPart = (DWORD)BULVERDE_BASE_REG_PA_MEMC;
    PhysAddr.HighPart = -1L;
    pdwMemCtrl = MmMapIoSpace(PhysAddr, (ULONG)PAGE_SIZE, (BOOLEAN)FALSE);
    if (pdwMemCtrl == NULL)
    {
        SMSC_WARNING0("Failed to get Virtual address of MemController\r\n");
    }
    else
    {
        SMSC_TRACE0(DBG_INIT, "Set Default Bus Timing for SMSC912x\r\n");
        // pdwMemCtrl[4]  : MSC2
        pdwMemCtrl[4] = pdwMemCtrl[4] & 0x0000FFFFUL;
        pdwMemCtrl[4] = pdwMemCtrl[4] | 0x8CD10000UL;   // 32bit as default
    }
    MmUnmapIoSpace(pdwMemCtrl, (ULONG)PAGE_SIZE);

#ifdef  SMSC_DIRECT_INTR
    RETAILMSG(1, (TEXT("Set GPIO44 to GPIO mode, Input and Falling Edge\r\n")));
    // Set GPIO 44 to GPIO mode, Input and Falling Edge
    PhysAddr.LowPart = (DWORD)BULVERDE_BASE_REG_PA_GPIO;
    PhysAddr.HighPart = -1L;
    pdwGpioCtrl = MmMapIoSpace(PhysAddr, (ULONG)PAGE_SIZE, (BOOLEAN)FALSE);
    if (pdwGpioCtrl == NULL)
    {
        SMSC_WARNING0("Failed to get Virtual address of Gpio\r\n");
    }
    else
    {
        // GPDR1 (0x010)
        pdwGpioCtrl[0x10/4] &= (~(1<<12));          // PD44 as Input
        // GAFR1_L (0x05C)
        pdwGpioCtrl[0x5C/4] &= (~(3<<24));          // PD44 as GPIO
        // GEDR1 (0x04C)
        pdwGpioCtrl[0x4C/4] = (1<<12);              // Clear any pending one
        // GFER1 (0x040)
        pdwGpioCtrl[0x40/4] |= (1<<12);             // PD44 as Falling Edge
        // GRER1 (0x034)
        pdwGpioCtrl[0x34/4] &= (~(1<<12));          // PD44 as No Rising Edge
    }

    // do not unmap pdwGpioCtrl
    // pdwGpioCtrl is used in PlatformEnableGpioInterrupt(void)
//  MmUnmapIoSpace(pdwGpioCtrl, (ULONG)PAGE_SIZE);
#endif

    SMSC_TRACE0(DBG_INIT, "+PlatformInitialize()\r\n");
}

void PlatformSetBusTiming(const DWORD dwChipIdReg)
{
    PHYSICAL_ADDRESS    PhysAddr;
    volatile PDWORD     pdwMemCtrl;
    DWORD               dwChipId;

    SMSC_TRACE0(DBG_INIT, "+PlatformSetBusTiming()\r\n");
    dwChipId = dwChipIdReg & 0xFFFF0000UL;
    if ((dwChipId == 0x01180000UL) ||
        (dwChipId == 0x01170000UL) ||
        (dwChipId == 0x118A0000UL) ||
        (dwChipId == 0x117A0000UL) ||
        (dwChipId == 0x92110000UL) ||
        (dwChipId == 0x92210000UL) ||
        (dwChipId == 0x218A0000UL) ||
        (dwChipId == 0x217A0000UL))
    {
        PhysAddr.LowPart = (DWORD)BULVERDE_BASE_REG_PA_MEMC;
        PhysAddr.HighPart = -1L;
        pdwMemCtrl = MmMapIoSpace(PhysAddr, (ULONG)PAGE_SIZE, (BOOLEAN)FALSE);
        if (pdwMemCtrl == NULL)
        {
            SMSC_WARNING0("Failed to get Virtual address of MemController\r\n");
        }
        else
        {
            // pdwMemCtrl[4]  : MSC2
            pdwMemCtrl[4] = pdwMemCtrl[4] & 0x0008FFFFUL;   // keep 16/32bit flag
            pdwMemCtrl[4] = pdwMemCtrl[4] | 0x83710000UL;
        }

        MmUnmapIoSpace(pdwMemCtrl, (ULONG)PAGE_SIZE);
    }
    SMSC_TRACE0(DBG_INIT, "-PlatformSetBusTiming()\r\n");
}

/* PlatformDisplayInfo:
 *   Will display info specific to the platform, such as
 *   processor registers, or DMA configurations
 */
void PlatformDisplayInfo()
{
    PDWORD      pPtr;
    BOOL        bRet;
    DWORD       msc2, mdrefr;
    DWORD       cccr, ccsr, clkcfg;
    DWORD       L, M, N2, r_clk, t_clk, s_clk, m_clk, k0db, k1db;

    SMSC_TRACE0(DBG_INIT, "+PlatformDisplayInfo()\r\n");
    pPtr = (PDWORD)VirtualAlloc((LPVOID)0, (DWORD)PAGE_SIZE, (DWORD)MEM_RESERVE, (DWORD)PAGE_NOACCESS); 

    bRet = VirtualCopy((PVOID)pPtr, (PVOID)(0x48000000UL>>8), 1024UL, (DWORD)(PAGE_READONLY | PAGE_PHYSICAL | PAGE_NOCACHE));
    if (bRet == TRUE) 
    {
        msc2 = pPtr[4];
        mdrefr = pPtr[1];
        SMSC_TRACE1(DBG_INIT, "MDREFR = 0x%08x\r\n", mdrefr);
RETAILMSG(1, (TEXT("MDREFR = 0x%08x\r\n"), mdrefr));
        SMSC_TRACE1(DBG_INIT, "MSC2 = 0x%08x\r\n", msc2);

    }
    else 
    {
        SMSC_WARNING1("Error! at VirtualCopy(). ErrNo = %d\r\n", GetLastError());
        bRet = VirtualFree((PVOID)pPtr, 0UL, (DWORD)MEM_RELEASE);
        return;
    }
    bRet = VirtualFree((PVOID)pPtr, 0UL, (DWORD)MEM_RELEASE);

    pPtr = (PDWORD)VirtualAlloc((LPVOID)0UL, (DWORD)PAGE_SIZE, (DWORD)MEM_RESERVE, (DWORD)PAGE_NOACCESS);   

    bRet = VirtualCopy((PVOID)pPtr, (PVOID)((DWORD)0x41300000UL>>8), 1024UL, (DWORD)(PAGE_READONLY | PAGE_PHYSICAL | PAGE_NOCACHE));
    if (bRet == TRUE) {
        cccr = pPtr[0];
        ccsr = pPtr[3];
        clkcfg = ReadCLKCFG();
        SMSC_TRACE1(DBG_INIT, "CCCR = 0x%08x\r\n", cccr);
        SMSC_TRACE1(DBG_INIT, "CCSR = 0x%08x\r\n", ccsr);
        SMSC_TRACE1(DBG_INIT, "CLKCFG = 0x%08x\r\n", clkcfg);

        L = ccsr & 0x1FUL;
        N2 = (ccsr >> 7) & 0x07UL;

        if (L <= 10UL) 
        {
            M = 1UL;
        }
        else if (L <= 20UL) 
        {
            M = 2UL;
        }
        else 
        {
            M = 4UL;
        }

        r_clk = (L * 13000000UL);
        t_clk = (L * 13000000UL * N2) / 2UL;

        if (clkcfg & 0x08UL)
        {
            s_clk = r_clk;      // Fast Clk Mode
        }
        else
        {
            s_clk = r_clk / 2UL;
        }

        if (cccr & 0x2000000UL)
        {
            m_clk = s_clk;      // Alt setting
        }
        else
        {
            m_clk = r_clk / M;
        }
        if (mdrefr & (1UL<<29))
        {
            k0db = 4UL;
        }
        else if (mdrefr & (1UL<<14))
        {
            k0db = 2UL;
        }
        else
        {
            k0db = 1UL;
        }

        if (mdrefr & (1UL<<17))
        {
            k1db = 2UL;
        }
        else
        {
            k1db = 1UL;
        }

        if (clkcfg & 0x1UL) 
        {
            SMSC_TRACE4(DBG_INIT, "Turbo Mode Clk: %d.%02dMHz (*%d.%d)\r\n",
                (ULONG)(t_clk/1000000UL), (ULONG)((t_clk%1000000UL)/10000UL), 
                (ULONG)(N2/2UL), (ULONG)((N2%2UL)*5UL));
        }
        else 
        {
            SMSC_TRACE3(DBG_INIT, "Run Mode Clk: %d.%02dMHz (*%d)\r\n", 
                (ULONG)(r_clk/1000000UL), (ULONG)((r_clk%1000000UL)/1000000UL), 
                L);
        }
        SMSC_TRACE5(DBG_INIT, "MemClk: %d.%02dMHz (Alt=%d, SDCLK[0]=/%d, SDCLK[1]=/%d\r\n", 
                (ULONG)(m_clk/1000000UL), 
                (ULONG)((m_clk % 1000000UL)/10000UL), 
                (ULONG)((cccr>>25UL)&1UL), k0db, k1db);
        SMSC_TRACE2(DBG_INIT, "SysClk: %d.%02dMHz\r\n", 
                (ULONG)(s_clk/1000000UL), 
                (ULONG)((s_clk%1000000UL)/10000UL));

        if (mdrefr & (1UL<<20UL))
        {
            SMSC_TRACE0(DBG_INIT, "APD is ON\r\n");
        }
        else
        {
            SMSC_TRACE0(DBG_INIT, "APD is OFF\r\n");
        }
    }
    else 
    {
        SMSC_WARNING1("Error! at VirtualCopy(). ErrNo = %d\r\n", GetLastError());
    }
    bRet = VirtualFree((PVOID)pPtr, 0UL, (DWORD)MEM_RELEASE);
    SMSC_TRACE0(DBG_INIT, "-PlatformDisplayInfo()\r\n");
}

BOOL DmaStartXfer(const DMA_XFER * const dmaXfer)
{
    const PBULVERDE_DMA_REG pDmaBase = (PBULVERDE_DMA_REG)(dmaXfer->DMABaseVA);
    const DWORD dwDmaCh = dmaXfer->dwDmaCh;
    DWORD dwDmaCmd;
    DWORD dwAlignMask;
    volatile DWORD dwLanPhysAddr, dwMemPhysAddr;

    // 1. validate the requested channel #
    if (dwDmaCh > 10UL)
    {
        SMSC_WARNING1("DmaStartXfer -- bad dwDmaCh=%ld\n", dwDmaCh);
        return FALSE;
    }

    // 2. make sure the channel's not already running
    if (!(pDmaBase->dcsr[dwDmaCh] & DCSR_STOPSTATE))
    {
        if (pDmaBase->dcsr[dwDmaCh] & DCSR_RUN)
        {
            SMSC_WARNING1("DmaStartXfer -- requested channel (%ld) is still running\n", dmaXfer->dwDmaCh);
            return FALSE;
        }
    }

    // 3. calculate the physical transfer addresses

    /*
     *  The code below assumes that the A11 address line is connected to the FIFO_SEL line in the 
     *  9118 device (thus the 0x800) for operation in fifo Direct Address mode.
     */
    dwMemPhysAddr = (DWORD)(dmaXfer->pdwBuf);
    dwLanPhysAddr = (DWORD)(dmaXfer->dwLanReg + 0x800UL);

    // 4. validate the address alignments
    if (dmaXfer->fClBurst)
    {
        // need CL alignment for CL bursts
        dwAlignMask = (CACHE_LINE_BYTES - 1UL);
    }
    else
    {
        // no burst, so DW alignment is OK
        dwAlignMask = 3UL;
    }

    if ((dwLanPhysAddr & dwAlignMask) != 0UL)
    {
        SMSC_WARNING2("DmaStartXfer -- bad dwLanPhysAddr (0x%08lX) alignment, fClBurst=%s\r\n", dwLanPhysAddr, dmaXfer->fClBurst?TEXT("TRUE"):TEXT("FALSE"));
        return FALSE;
    }

    if ((dwMemPhysAddr & 0x03UL) != 0UL)
    {
        SMSC_WARNING2("DmaStartXfer -- bad dwMemPhysAddr (0x%08lX) alignment, fClBurst=%s\r\n", dwMemPhysAddr, dmaXfer->fClBurst?TEXT("TRUE"):TEXT("FALSE"));
        return FALSE;
    }

    // 5. validate the transfer size & alignment
    if (dmaXfer->dwDwCnt >= 8192UL)
    {
        SMSC_WARNING1("DmaStartXfer -- dwDwCnt =%ld is too big\r\n", dmaXfer->dwDwCnt);
        return FALSE;
    }

    pDmaBase->dcsr[dwDmaCh] = DCSR_NODESC;

    dwDmaCmd = 0x00UL;
    // Select correct Ch and set SRC, DST and counter
    if (dmaXfer->fMemWr == TRUE)
    {
        // Read from 118
        // Set Source and destination addresses
        pDmaBase->ddg[dwDmaCh].dtadr = (UINT)dwMemPhysAddr;
        pDmaBase->ddg[dwDmaCh].dsadr = (UINT)dwLanPhysAddr;

        dwDmaCmd |= (DWORD)DCMD_INCTRGADDR;
        dwDmaCmd |= (DWORD)DCMD_INCSRCADDR;
    }
    else
    {
        // Write to 118
        // Set Source and destination addresses
        pDmaBase->ddg[dwDmaCh].dtadr = (UINT)dwLanPhysAddr;
        pDmaBase->ddg[dwDmaCh].dsadr = (UINT)dwMemPhysAddr;

        dwDmaCmd |= DCMD_INCSRCADDR;
        dwDmaCmd |= DCMD_INCTRGADDR;
    }

    // Set the transmit size in terms of the xfer mode
    if (dmaXfer->fClBurst == TRUE)
    {
        dwDmaCmd |= DCMD_BURST32;
    }
    else
    {
        dwDmaCmd |= DCMD_BURST8;
    }

    dwDmaCmd |= (DCMD_LENGTH & (dmaXfer->dwDwCnt << 2UL));

    pDmaBase->ddg[dwDmaCh].dcmd = (UINT)dwDmaCmd;
    pDmaBase->dcsr[dwDmaCh] |= DCSR_RUN;
    // DMA Transfering....
    if (pDmaBase->dcsr[dwDmaCh] & DCSR_BUSERR) {
        SMSC_WARNING0("DMA BusError!\r\n");
    }

    return TRUE;
}

BOOL DmaInitialize(NDIS_HANDLE hMiniportAdapterContext)
{
    CPCSMSC9118_ADAPTER pAdapter = (PSMSC9118_ADAPTER)(hMiniportAdapterContext);

    // Make Lint Happy
    hMiniportAdapterContext = hMiniportAdapterContext;

    SMSC_TRACE0(DBG_DMA,"+DmaInitialize()\n");
    SMSC_ASSERT(pAdapter);
    SMSC_TRACE0(DBG_DMA,"-DmaInitialize()\n");
    return TRUE;
}

DWORD DmaGetDwCnt(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh)
{
    const volatile BULVERDE_DMA_REG * const pDmaBase = (PBULVERDE_DMA_REG)(dmaXfer->DMABaseVA);

    return ((DCMD_LENGTH & (DWORD)pDmaBase->ddg[dwDmaCh].dcmd) >> 2UL);
}

void DmaDisable(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh)
{
    BULVERDE_DMA_REG * const pDmaBase = (PBULVERDE_DMA_REG)(dmaXfer->DMABaseVA);

    pDmaBase->dcsr[dwDmaCh] &= ~DCSR_RUN;
    while (!(pDmaBase->dcsr[dwDmaCh] & DCSR_STOPSTATE))
    {
    }
}

void DmaComplete(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh) 
{
    DWORD   dwTimeOut = 1000UL;

    while ((DmaGetDwCnt(dmaXfer, dwDmaCh)) && (dwTimeOut))
    {
        SMSC_MICRO_DELAY(1U);
        dwTimeOut--;
    }

    if (dwTimeOut == 0UL) {
        SMSC_WARNING0("DmaComplete: Timed out\n");
    }
    
    DmaDisable(dmaXfer, dwDmaCh);
}

void CleanCacheLine(DWORD dwAddr);
void DrainWriteBuffers(void);
ULONG GetPID(void);

VOID BufferCacheFlush(const void * const pucBufAddress, const DWORD uiBufLen)
{
    DWORD   dwCurrAddr, dwEndAddr, dwLinesToGo;

    dwCurrAddr = (DWORD)pucBufAddress & ~(CACHE_LINE_BYTES-1UL);
    dwEndAddr = (((DWORD)pucBufAddress) + uiBufLen + CACHE_LINE_BYTES) & ~(CACHE_LINE_BYTES-1UL);

    dwLinesToGo = (dwEndAddr - dwCurrAddr) / CACHE_LINE_BYTES;
    while (dwLinesToGo)
    {
        CleanCacheLine(dwCurrAddr+GetPID());
        dwCurrAddr += CACHE_LINE_BYTES;
        dwLinesToGo--;
    }
    DrainWriteBuffers();
}

void PlatformEnableGpioInterrupt(void)
{
#ifdef  SMSC_DIRECT_INTR
    if (pdwGpioCtrl != NULL)
    {
        // GFER1 (0x40)
        pdwGpioCtrl[0x40/4] |= (1<<12);             // GPIO 44 as Falling Edge
    }
#endif
}
