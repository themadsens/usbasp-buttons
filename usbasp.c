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

#define LEDG_PIN    SBIT(PORTC, PC1)
#define LEDG_OUT    SBIT(DDRC,  PC1)
#define LEDR_PIN    SBIT(PORTC, PC0)
#define LEDR_OUT    SBIT(DDRC,  PC0)

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

u8 ledR = 0;
u8 ledG = 0;
int deciMilli = 0;
u32 milliSecs = 0;
ISR(TIMER0_OVF_vect)
{
	deciMilli++;
    TCNT0 = 255 - TIMERTOP;
    LEDR_PIN = deciMilli < 2 && ledR ? 0 : 1;
    LEDG_PIN = deciMilli < 2 && ledG ? 0 : 1;
}

uchar line[50];
uchar *linep;
extern void setup(void)
{
  linep = NULL;
  memset(line, 0, sizeof(line));
  milliSecs = 0;

  TCCR0 = BIT(CS01);    // Timer0 /8 clock prescaler
  SBIT(TIMSK, TOIE0) = 1;
  LEDG_OUT = 1;
  LEDG_PIN = 1;
  LEDR_OUT = 1;
  LEDR_PIN = 1;
}

struct { u16 d1, d2; } ledCnt[4];
static void initPins()
{
  memset(ledCnt, 0, sizeof(ledCnt));
  ledR = 1;
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

u32 csCnt = 0;
char str[60];
i16 btnTime = 0;
u16 btnMask = 0;
u16 btnPrev = 0;

#define SPRINT(fmt, ...) (sprintf_P(str, PSTR(fmt "\r\n"), ##__VA_ARGS__), str)
extern void repeat(void)
{
  csCnt++;
  if (deciMilli >= 10) {
    deciMilli -= 10;
    milliSecs++;
  }
  else {
    return;
  }

  if (0 == (milliSecs % 500)) {
    ledG = ~ledG;
  }
  if (0 == (milliSecs % 5000)) {
    usbPuts(SPRINT("HELLO %10lu %lu %d '%s' %hu %04hx", milliSecs, csCnt, linep?linep-line:0, line, btnMask, btnTime));
  }
  csCnt = 0;

  // Handle buttons
  int i, mask;
  for (i = 0, mask=0; i < 3; i++) {
    int b;
    switch (i) {
     case 0: b = BTN1_PIN==0; break;
     case 1: b = BTN2_PIN==0; break;
     case 2: b = BTN3_PIN==0; break;
    }
    mask |= (b?1:0) << i;
  }
  if (mask != btnMask) {
    btnMask = mask;
    btnTime = milliSecs & 0xffff;
  }
  else if ((i16)(milliSecs & 0xffff) - btnTime > 50 && btnMask != btnPrev) {
    for (i = 0; i < 3; i++) {
      if ((btnMask & (1<<i)) != (btnPrev & (1<<i)))
        usbPuts(SPRINT("BUTTON%d=%d", i+1, (btnMask & (1<<i)) ? 1 : 0));
    }
    btnPrev = btnMask;
  }

  do {
    int ch = usbGetch();
    if (ch < 0)
      return;
    linep = linep == NULL ? line : linep + 1;
    linep[0] = ch;
    linep[1] = '\0';
  } while (*linep != '\r' && *linep != '\n' && linep != line + sizeof(line) - 1);

  *linep = '\0';
  linep = NULL;

  if (0 == strcasecmp((char*)line, "init"))
      initPins();
  else {
    usbPuts(SPRINT("ERROR '%s'", line));
    *line = '\0';
    return;
  }
  usbPuts(SPRINT("OK '%s'", line));
  *line = '\0';
}

// vim: set sw=2 sts=2 et: