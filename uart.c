/* Name: uart.c
 * Project: AVR USB driver for CDC interface on Low-Speed USB
 * Author: Osamu Tamura
 * Creation Date: 2006-06-18
 * Tabsize: 4
 * Copyright: (c) 2006 by Recursion Co., Ltd.
 * License: Proprietary, free under certain conditions. See Documentation.
 *
 * 2006-07-08   adapted to higher baudrate by T.Kitazawa
 */
/*
General Description:
    This module implements the UART rx/tx system of the USB-CDC driver.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>   /* needed by usbdrv.h */
#include "oddebug.h"
#include "usbdrv.h"
#include "uart.h"

extern uchar    sendEmptyFrame;

/* UART buffer */
uchar    urptr, uwptr, irptr, iwptr;
uchar    rx_buf[RX_SIZE+HW_CDC_BULK_IN_SIZE], tx_buf[TX_SIZE];


void uartInit(ulong baudrate, uchar parity, uchar stopbits, uchar databits)
{
    setup();
}

int usbGetch()
{
    if (uwptr == irptr)
        return -1;

    int ch = tx_buf[irptr];
    irptr = (irptr+1) & TX_MASK;

    return ch;
}

int usbPutch(uchar ch)
{
    uchar next = (iwptr+1) & RX_MASK;
    if (next == urptr)
        return -1;

    rx_buf[iwptr] = ch;
    iwptr = next;
    return 0;
}

void doUsbTx(void )
{
    /*  USB <= device  */
    if( usbInterruptIsReady() && (iwptr!=urptr || sendEmptyFrame) ) {
        uchar   bytesRead, i;

        bytesRead = (iwptr-urptr) & RX_MASK;
        if(bytesRead>HW_CDC_BULK_IN_SIZE)
            bytesRead = HW_CDC_BULK_IN_SIZE;
        uchar next    = urptr + bytesRead;
        if( next>=RX_SIZE ) {
            next &= RX_MASK;
            for( i=0; i<next; i++ )
                rx_buf[RX_SIZE+i]   = rx_buf[i];
        }
        usbSetInterrupt(rx_buf+urptr, bytesRead);
        urptr = next;

        /* send an empty block after last data block to indicate transfer end */
        sendEmptyFrame = (bytesRead==HW_CDC_BULK_IN_SIZE && iwptr==urptr)? 1:0;
    }
}

void uartPoll(void)
{
    doUsbTx();
    repeat();
}

