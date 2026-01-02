#define uint32_t unsigned int
#define mmio_write(reg, data)  *(volatile uint32_t*)(reg) = (data)
#define mmio_read(reg)  *(volatile uint32_t*)(reg)

#define GPIO_OTG_TRANS                  0
#define GPIO_BATTERY_CHARGING           1
#define GPIO_KEYPAD_BACKLIGHT           2
#define GPIO_DISPLAY_BACKLIGHT          3
#define GPIO_HEADSET_DETECT             4
#define GPIO_NAND_READY                 5
#define GPIO_ROLLER_TRIG                6
#define GPIO_ROLLER_DIR                 7
#define GPIO_BATTERY_CHG_EN             8
#define GPIO_BATTERY_CHG_HI             9
#define GPIO_VIBRATOR_EN				10
#define GPIO_PNDTBUS_DATA				11
#define GPIO_PNDTBUS_CLK				12

#define GPIO_COUNT                      13

#define IRQ_GPIO_0						32

#define GPIO_DIR_INPUT          0x01
#define GPIO_DIR_OUTPUT         0x00
#define GPIO_INT_LOW_HIGH       0x02
#define GPIO_INT_HIGH_LOW       0x00

#define SYSINTR_LAN9115 SYSINTR_FIRMWARE + 11
#define GPIO_DEVICE_NAME        L"GIO1:"

#define IOCTL_GPIO_SETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_CLRBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_UPDATEBIT    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETIRQ       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0306, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DDK_GET_DRIVER_IFC        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0100, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
	DWORD context;
	VOID(*pfnSetBit)(DWORD context, DWORD id);
	VOID(*pfnClrBit)(DWORD context, DWORD id);
	VOID(*pfnUpdateBit)(DWORD context, DWORD id, DWORD value);
	DWORD(*pfnGetBit)(DWORD context, DWORD id);
	VOID(*pfnSetMode)(DWORD context, DWORD id, DWORD mode);
	DWORD(*pfnGetMode)(DWORD context, DWORD id);
	DWORD(*pfnGetIrq)(DWORD context, DWORD id);
} DEVICE_IFC_GPIO;

typedef struct {
	DEVICE_IFC_GPIO ifc;
	HANDLE hDevice;
} DEVICE_CONTEXT_GPIO;

DEFINE_GUID(
	DEVICE_IFC_GPIO_GUID, 0xa0272611, 0xdea0, 0x4678,
	0xae, 0x62, 0x65, 0x61, 0x5b, 0x7d, 0x53, 0xaa
	);

extern const DWORD g_gpioMap[GPIO_COUNT];

typedef struct {
	DWORD       cookie;
	int         nbGpioGrp;
	UINT        *rgRanges;
	HANDLE      *rgHandles;
	DEVICE_IFC_GPIO **rgGpioTbls;
	WCHAR       **name;
} GpioDevice_t;

enum
{
	GIC_BASE = 0x92000000,
 
    GICD_CTLR = 0x1000,
	GICD_ISENABLER0 = 0x1100,
	GICD_ICENABLER0 = 0x1180,
	GICD_ITARGETSR0 = 0x1800,
	GICD_ICFGR0 = 0x1C00,
	GICD_SGIR   = 0x1F00,

	GICC_CTLR = 0x2000,
	GICC_PMR = 0x2004,
	GICC_IAR = 0x200C,
	GICC_EOIR = 0x2010,
	GICC_DIR = 0x3000
};

enum
{
	SP804_BASE  = 0x91110000,
	SP804_BASE2 = 0x91110020,

    TimerLoad    = 0x00,
	TimerValue   = 0x04,
	TimerControl = 0x08,
	TimerIntClr  = 0x0C,
	TimerRIS     = 0x10,
	TimerMIS     = 0x14,
	TimerBGLoad  = 0x18
};

enum
{
	LCD_BASE  = 0x911f0000,
	VRAM_ADDR = 0x18000000,

	LCDTiming0 = 0x000,
	LCDTiming1 = 0x004,
	LCDUpbase  = 0x010,
	LCDControl = 0x018
};

#define TIMER_IRQ 34

enum
{
	RTC_BASE = 0x91170000,

	RTCDR   = 0x000,
	RTCMR   = 0x004,
	RTCLR   = 0x008,
	RTCCR   = 0x00C,
	RTCIMSC = 0x010,
	RTCRIS  = 0x014,
	RTCMIS  = 0x018,
	RTCICR  = 0x01C
};

#define RTC_IRQ 36
#define IPI_IRQ 1

enum
{
	// we need to use mapped address
	UART0_BASE = (0x91090000),
 
    // The offsets for each register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C)
};

void gic_irq_enable(int irq);
void gic_irq_disable(int irq);

__inline HANDLE GPIOOpen()
{
	HANDLE hDevice;
	DEVICE_CONTEXT_GPIO *pContext = NULL;

	hDevice = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) goto clean;

	// Allocate memory for our handler...
	if ((pContext = (DEVICE_CONTEXT_GPIO *)LocalAlloc(
		LPTR, sizeof(DEVICE_CONTEXT_GPIO)
		)) == NULL) {
		CloseHandle(hDevice);
		goto clean;
	}

	// Get function pointers, fail when IOCTL isn't supported...
	if (!DeviceIoControl(
		hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_GPIO_GUID,
		sizeof(DEVICE_IFC_GPIO_GUID), &pContext->ifc, sizeof(DEVICE_IFC_GPIO),
		NULL, NULL
		)) {
		CloseHandle(hDevice);
		LocalFree(pContext);
		pContext = NULL;
		goto clean;
	}

	// Save device handle
	pContext->hDevice = hDevice;

clean:
	return pContext;
}

__inline VOID GPIOClose(HANDLE hContext)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	CloseHandle(pContext->hDevice);
	LocalFree(pContext);
}

__inline VOID GPIOSetBit(HANDLE hContext, DWORD id)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	pContext->ifc.pfnSetBit(pContext->ifc.context, id);
}

__inline VOID GPIOClrBit(HANDLE hContext, DWORD id)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	pContext->ifc.pfnClrBit(pContext->ifc.context, id);
}

__inline VOID GPIOUpdateBit(HANDLE hContext, DWORD id, DWORD value)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	pContext->ifc.pfnUpdateBit(pContext->ifc.context, id, value);
}

__inline DWORD GPIOGetBit(HANDLE hContext, DWORD id)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	return pContext->ifc.pfnGetBit(pContext->ifc.context, id);
}

__inline VOID GPIOSetMode(HANDLE hContext, DWORD id, DWORD mode)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	pContext->ifc.pfnSetMode(pContext->ifc.context, id, mode);
}

__inline DWORD GPIOGetMode(HANDLE hContext, DWORD id)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	return pContext->ifc.pfnGetMode(pContext->ifc.context, id);
}

__inline DWORD GPIOGetIrq(HANDLE hContext, DWORD id)
{
	DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
	return pContext->ifc.pfnGetIrq(pContext->ifc.context, id);
}