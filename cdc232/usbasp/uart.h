
/* Name: uart.h
 * Project: AVR USB driver for CDC interface on Low-Speed USB
 * Author: Osamu Tamura
 * Creation Date: 2006-06-18
 * Tabsize: 4
 * Copyright: (c) 2006 by Recursion Co., Ltd.
 * License: Proprietary, free under certain conditions. See Documentation.
 */

#ifndef __uart_h_included__
#define __uart_h_included__

#include <avr/io.h> /* for TXEN or TXEN0 resp., if available */

#ifndef uchar
#define uchar   unsigned char
#endif

#ifndef ulong
#define ulong   unsigned long
#endif

#define HW_CDC_BULK_OUT_SIZE     8
#define HW_CDC_BULK_IN_SIZE      8


#define	RX_SIZE		128     /* UART receive buffer size (must be 2^n, 16-128)  */
#define	TX_SIZE		64      /* UART transmit buffer size (must be 2^n, 16-256) */
#define	RX_MASK		(RX_SIZE-1)
#define	TX_MASK		(TX_SIZE-1)

/* ------------------------------------------------------------------------- */
/*	---------------------- Type Definition --------------------------------- */
/* ------------------------------------------------------------------------- */
typedef union usbDWord {
    ulong	dword;
    uchar   bytes[4];
} usbDWord_t;


extern uchar    urptr, uwptr, irptr, iwptr;
extern uchar    rx_buf[], tx_buf[]; 

// From main()
extern void uartInit(ulong baudrate, uchar parity, uchar stopbits, uchar databits);
extern void uartPoll(void);

// From repeat()
extern int usbGetch();
extern int usbPutch(uchar ch);

// Arduino like
extern void setup(void);
extern void repeat(void);

/* The following function returns the amount of bytes available in the TX
 * buffer before we have an overflow.
 */
static inline uchar uartTxBytesFree(void)
{
    return (irptr - uwptr - 1) & TX_MASK;
}


#endif  /*  __uart_h_included__  */

