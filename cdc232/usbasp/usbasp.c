/**
 * @file usbasp.c
 * ABSTRACT HERE << 
 *
 * $Id$
 *
 * (C) Copyright 2018 MadsenSoft, fm@madsensoft.dk
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>   /* needed by usbdrv.h */
#include "oddebug.h"
#include "usbdrv.h"
#include "uart.h"
#include "util.h"

#define LED0_PIN    SBIT(PORTC, PC2)
#define LED0_OUT    SBIT(DDRC,  PC2)
#define LED1_PIN    SBIT(PORTB, PB2)
#define LED1_OUT    SBIT(DDRB,  PB2)
#define LED2_PIN    SBIT(PORTB, PB4)
#define LED2_OUT    SBIT(DDRB,  PB4)
#define LED3_PIN    SBIT(PORTB, PB5)
#define LED3_OUT    SBIT(DDRB,  PB5)

#define BTN1_PIN SBIT(PINB,  PB3)
#define BTN1_PU  SBIT(PORTB, PB3)
#define BTN2_PIN SBIT(PIND,  PD1)
#define BTN2_PU  SBIT(PORTD, PD1)
#define BTN3_PIN SBIT(PIND,  PD0)
#define BTN3_PU  SBIT(PORTD, PD0)

#define TIMERRATE     10000
#define TIMERTOP  F_CPU / TIMERRATE / 8

int usbPuts (char *s)
{
  while (*s)
    usbPutch(*s++);
  return 0;
}

int deciMilli = 0;
u32 milliSecs = 0;
ISR(TIMER0_OVF_vect)
{
	deciMilli++;
    TCNT0 = 255 - TIMERTOP;
}

uchar line[50];
uchar *linep;
extern void setup(void)
{
  linep = line;

  TCCR0 = BIT(CS01);    // Timer0 /8 clock prescaler
  SBIT(TIMSK, TOIE0) = 1;
}

struct { u16 d1, d2; } ledCnt[4];
static void initPins()
{
  memset(ledCnt, 0, sizeof(ledCnt));
  LED0_OUT = 1;
  LED1_OUT = 1;
  LED2_OUT = 1;
  LED3_OUT = 1;

  LED0_PIN = 0;
  LED1_PIN = 0;
  LED2_PIN = 0;
  LED3_PIN = 0;

  BTN1_PU = 1;
  BTN2_PU = 1;
  BTN3_PU = 1;
}

int csCnt = 0;
char str[60];
#define SPRINT(fmt, ...) (sprintf(str, fmt "\n", ##__VA_ARGS__), str)
extern void repeat(void)
{
  csCnt++;
  if (deciMilli >= 10) {
    deciMilli -= 10;
    milliSecs++;
  }
  else
    return;

  if (0 == (milliSecs % 10000)) {
    usbPuts(SPRINT("Hello world %d", csCnt));
  }
  csCnt = 0;

  do {
    int ch = usbGetch();
    if (ch < 0)
      return;
    linep = linep == NULL ? linep : linep + 1;
    *linep = ch;
  } while (*linep != '\r' && *linep != '\n' && linep != line + sizeof(line) - 1);

  *linep = '\0';

  if (0 == strcmp(line, "init"))
      initPins;

}

// vim: set sw=2 sts=2 et:
