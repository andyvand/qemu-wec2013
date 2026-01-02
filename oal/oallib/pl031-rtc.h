#ifndef SRC_DEVICES_RTC_DRIVERS_PL031_RTC_PL031_RTC_H_
#define SRC_DEVICES_RTC_DRIVERS_PL031_RTC_PL031_RTC_H_

/*
* Register definitions
 */
#define RTC_DR      0x00    /* Data read register */
#define RTC_MR      0x04    /* Match register */
#define RTC_LR      0x08    /* Data load register */
#define RTC_CR      0x0c    /* Control register */
#define RTC_IMSC    0x10    /* Interrupt mask and set register */
#define RTC_RIS     0x14    /* Raw interrupt status register */
#define RTC_MIS     0x18    /* Masked interrupt status register */
#define RTC_ICR     0x1c    /* Interrupt clear register */
/* ST variants have additional timer functionality */
#define RTC_TDR     0x20    /* Timer data read register */
#define RTC_TLR     0x24    /* Timer data load register */
#define RTC_TCR     0x28    /* Timer control register */
#define RTC_YDR     0x30    /* Year data read register */
#define RTC_YMR     0x34    /* Year match register */
#define RTC_YLR     0x38    /* Year data load register */

#define RTC_CR_CWEN (1 << 26)   /* Clockwatch enable bit */

#define RTC_TCR_EN  (1 << 1) /* Periodic timer enable bit */

/* Common bit definitions for Interrupt status and control registers */
#define RTC_BIT_AI  (1 << 0) /* Alarm interrupt bit */
#define RTC_BIT_PI  (1 << 1) /* Periodic interrupt bit. ST variants only. */

/* Common bit definations for ST v2 for reading/writing time */
#define RTC_SEC_SHIFT 0
#define RTC_SEC_MASK (0x3F << RTC_SEC_SHIFT) /* Second [0-59] */
#define RTC_MIN_SHIFT 6
#define RTC_MIN_MASK (0x3F << RTC_MIN_SHIFT) /* Minute [0-59] */
#define RTC_HOUR_SHIFT 12
#define RTC_HOUR_MASK (0x1F << RTC_HOUR_SHIFT) /* Hour [0-23] */
#define RTC_WDAY_SHIFT 17
#define RTC_WDAY_MASK (0x7 << RTC_WDAY_SHIFT) /* Day of Week [1-7] 1=Sunday */
#define RTC_MDAY_SHIFT 20
#define RTC_MDAY_MASK (0x1F << RTC_MDAY_SHIFT) /* Day of Month [1-31] */
#define RTC_MON_SHIFT 25
#define RTC_MON_MASK (0xF << RTC_MON_SHIFT) /* Month [1-12] 1=January */

#define RTC_TIMER_FREQ 32768

typedef struct pl031_local {
  void *base;
} pl031_local;

typedef struct Pl031Regs {
  unsigned int dr;
  unsigned int mr;
  unsigned int lr;
  unsigned int cr;
  unsigned int msc;
  unsigned int ris;
  unsigned int mis;
  unsigned int icr;
} Pl031Regs;

#endif /* SRC_DEVICES_RTC_DRIVERS_PL031_RTC_PL031_RTC_H_ */

