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
/*lint -e726 -e760 -e761 -e762 -e763 -e767 -e773 -e783 -e793 -e806 -e828 -e912*/
/*lint -e935 -e937 -e950 -e955 -e956 -e957 -e958 -e959 -e960 -e961 -e962*/
/*lint -e973 -e1916 */
#include <bulverde_base_regs.h>
#include <bulverde_gpio.h>
#include <bulverde_dma.h>
#include <bulverde_intr.h>
#include <bulverde_memctrl.h>
#include <bulverde_ost.h>
/*lint -restore*/

//#define   SMSC_DIRECT_INTR    // GPIO Edge-Triggered Direct Interrupt (SMSC9118.IRQ --> GPIO, no intervening CPLD)

#define CACHE_LINE_BYTES                32UL    //Cache size = 32bytes

#define RX_END_ALIGNMENT                (CACHE_LINE_BYTES)
#define TX_BURST_DMA_END_ALIGNMENT      (CACHE_LINE_BYTES)
#define INT_DEAS                        0x16000000UL

#define DMAC_REGBASE                    (BULVERDE_BASE_REG_PA_DMAC)
#define DMAC_REGSIZE                    (0x400)

#define DCSR_RUN        (1U<<31)
#define DCSR_NODESC     (1U<<30)
#define DCSR_STOPIRQEN  (1U<<29)
#define DCSR_REQPEND    (1U<<8)
#define DCSR_STOPSTATE  (1U<<3)
#define DCSR_ENDINTR    (1U<<2)
#define DCSR_STARTINTR  (1U<<1)
#define DCSR_BUSERR     (1U<<0)

#define DCMD_INCSRCADDR (1UL<<31)
#define DCMD_INCTRGADDR (1UL<<30)
#define DCMD_FLOWSRC    (1UL<<29)
#define DCMD_FLOWTRG    (1UL<<28)
#define DCMD_STARTIRQEN (1UL<<22)
#define DCMD_ENDIRQEN   (1UL<<21)
#define DCMD_WORDSWAP   (1UL<<18)
#define DCMD_BURST8     (1UL<<16)
#define DCMD_BURST16    (2UL<<16)
#define DCMD_BURST32    (3UL<<16)
#define DCMD_WIDTH      (1UL<<14)
#define DCMD_WIDTH2     (2UL<<14)
#define DCMD_WIDTH4     (3UL<<14)
#define DCMD_LENGTH     (0x1FFFUL)

//DMA Transfer structure
typedef struct _DMA_XFER 
{
    DWORD DMABaseVA;
    DWORD dwLanReg;
    DWORD *pdwBuf;  //PA
    DWORD dwDmaCh;
    DWORD dwDwCnt;

    BOOLEAN fMemWr;
    BOOLEAN fClBurst;
    BOOLEAN fLanAddrInc;
    BOOLEAN fMemAddrInc;
    BOOLEAN fReserve;

    // Make Lint happy
    BYTE    padding[3];
} DMA_XFER, *PDMA_XFER;

#define DMA_INVALID_COUNT (0xFFFFFFFFUL)
#define DMA_RX_CH   2UL
#define DMA_TX_CH   3UL

BOOL DmaInitialize(NDIS_HANDLE hMiniportAdapterContext);
BOOL DmaStartXfer(const DMA_XFER * const dmaXfer);
DWORD DmaGetDwCnt(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh);
VOID DmaDisable(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh);
VOID DmaComplete(const DMA_XFER * const dmaXfer, const DWORD dwDmaCh);
VOID BufferCacheFlush(const void * const pStartAddress, const DWORD dwLengthInBytes);
void PlatformInitialize(void);
void PlatformDisplayInfo(void);
void PlatformSetBusWidth(const DWORD dwBusWidth);
void PlatformSetBusTiming(const DWORD dwChipIdReg);
void PlatformEnableGpioInterrupt(void);
