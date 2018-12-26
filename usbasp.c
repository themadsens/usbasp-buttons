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
#include "util.h"

#define USBINTR_READSTR 1

#define USBCMD_INIT     1
#define USBCMD_SETLED   2
#define USBCMD_SCROLL   3
#define USBCMD_FETCHSTR 4

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

char *usbStr;
int usbIntr (char *s, u8 btn, u8 chg)
{
#if 1
  usbStr = s;
  u8 cmd[3] = { USBINTR_READSTR, btn, chg };
  usbSetInterrupt(cmd, 3);
#endif
  return 0;
}
char str[90];
#define SPRINT(fmt, ...) (sprintf_P(str, PSTR(fmt "\r\n"), ##__VA_ARGS__), str)

u8 ledR = 1;
u8 ledG = 0;
u8 led1 = 0;
u8 led2 = 0;
u8 led3 = 0;
int deciMilli = 0;
u32 milliSecs = 0;
ISR(TIMER0_OVF_vect)
{
	deciMilli++;
    TCNT0 = 255 - TIMERTOP;
    LEDR_PIN = deciMilli < 2 && ledR ? 0 : 1;
    LEDG_PIN = deciMilli < 2 && ledG ? 0 : 1;
    LED1_PIN = deciMilli < 2 && led1 ? 1 : 0;
    LED2_PIN = deciMilli < 2 && led2 ? 1 : 0;
    LED3_PIN = deciMilli < 2 && led3 ? 1 : 0;
}

extern void setup(void)
{
  milliSecs = 0;

  TCCR0 = BIT(CS01);    // Timer0 /8 clock prescaler
  SBIT(TIMSK, TOIE0) = 1;
  LEDG_OUT = 1;
  LEDG_PIN = 1;
  LEDR_OUT = 1;
  LEDR_PIN = 1;

  BTN1_PU = 1;
  BTN2_PU = 1;
  BTN3_PU = 1;
}

struct { u16 p1, p2; } ledCnt[4];
int initialised = 0;
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

  initialised = 1;
}

int scrollMode = 0;
void handleLed()
{
  if (!initialised)
    return;

  int m = 0, i;
  if (scrollMode)
    m = 1 << (milliSecs % 1000 / 250);
  else {
    for (i = 0; i < 4; i++) {
      if (ledCnt[i].p1 == 1)
        m |= 1 << i;
      else if (ledCnt[i].p1 == 0)
        ;
      else if (ledCnt[i].p2 == 0 && (milliSecs % (ledCnt[i].p1 * 2)) < ledCnt[i].p1)
        m |= 1 << i;
      else if (ledCnt[i].p2 != 0 && (milliSecs % (ledCnt[i].p1 + ledCnt[i].p2)) < ledCnt[i].p1)
        m |= 1 << i;
    }
  }
  LED0_PIN = m&1 ? 1:0;
  led1 = m&2 ? 1:0;
  led2 = m&4 ? 1:0;
  led3 = m&8 ? 1:0;
}


// Handle buttons
i16 btnTime = 0;
u16 btnMask = 0;
u16 btnPrev = 0;

void handleButtons()
{
#if 0
  if (!initialised)
    return;
#endif

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
    int btnChng = btnMask ^ btnPrev;
    usbIntr(SPRINT("BTN %d,%d,%d CHG %d,%d,%d", btnMask&1, (btnMask>>1)&1, (btnMask>>2)&1,
                                                btnChng&1, (btnChng>>1)&1, (btnChng>>2)&1), btnMask, btnChng);
    btnPrev = btnMask;
    ledR = btnMask != 0;
    if (!initialised) {
      initPins();
      ledCnt[0].p1 = 50;
      ledCnt[0].p2 = 1950;
    }
  }
}

extern void repeat(void)
{
  int msIn = deciMilli;
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

  handleButtons();
  handleLed();


  if (0 == (milliSecs % 5000)) {
    usbIntr(SPRINT("HELLO %10lu %lu %hu %d/%d %d,%d,%d", milliSecs, btnMask, deciMilli, msIn,
                                                        ledCnt[0].p1, ledCnt[0].p2, (milliSecs % (ledCnt[0].p1 * 2))), 0, 0);
  }
}

uchar   usbFunctionSetup(u8 *setupData)
{
  usbRequest_t *rq = (usbRequest_t *) setupData;

  if (USBCMD_FETCHSTR == (rq->bRequest & 0xf)) {
    usbMsgPtr = (u8*)usbStr;
    return strlen(usbStr);
  }
  else if (USBCMD_INIT == (rq->bRequest & 0xf)) {
    initPins();
  }
  else if (USBCMD_SCROLL == (rq->bRequest & 0xf)) {
    scrollMode = rq->bRequest >> 4;
  }
  else if (USBCMD_SETLED == (rq->bRequest & 0xf)) {
    int led = rq->bRequest >> 4;
    ledCnt[led].p1 = rq->wValue.bytes[1] * 0x100 + rq->wValue.bytes[0];
    ledCnt[led].p2 = rq->wIndex.bytes[1] * 0x100 + rq->wIndex.bytes[0];
  }
  return 0;
}


int main() {
  //odDebugInit();
  uchar i, j;
  cli();
  usbDeviceDisconnect();
  for (j = 0; --j;) {
    for (i = 0; --i;)
      ;
  }
  usbDeviceConnect();
  usbInit();

  setup();
  sei();
  for (;;) {
    usbPoll();
    repeat();
  }
}

// vim: set sw=2 sts=2 et:
