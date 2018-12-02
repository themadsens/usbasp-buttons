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
char str[90];
#define SPRINT(fmt, ...) (sprintf_P(str, PSTR(fmt "\r\n"), ##__VA_ARGS__), str)

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

char line[50];
char *linep;
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

  BTN1_PU = 1;
  BTN2_PU = 1;
  BTN3_PU = 1;

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
  LED1_PIN = m&2 ? 1:0;
  LED2_PIN = m&4 ? 1:0;
  LED3_PIN = m&8 ? 1:0;
}

void handleLedCmd(int led, const char *par)
{
  char *cp = strchr(par, ',') ? strchr(par, ',') + 1 : NULL;
  ledCnt[led].p1 = ledCnt[led].p2 = 0;
  while (*par >= '0' && *par <= '9')
    ledCnt[led].p1 = ledCnt[led].p1 * 10 + (*par++) - '0';
  while (cp && *cp >= '0' && *cp <= '9')
    ledCnt[led].p2 = ledCnt[led].p2 * 10 + (*cp++) - '0';
}

// Handle buttons
u32 csCnt = 0;
i16 btnTime = 0;
u16 btnMask = 0;
u16 btnPrev = 0;

void handleButtons()
{
  if (!initialised)
    return;

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
}

extern void repeat(void)
{
  int msIn = deciMilli;
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

  handleButtons();
  handleLed();

  int ch;
  do {
    ch = usbGetch();
    if (ch < 0)
      break;
    linep = linep == NULL ? line : linep + 1;
    if (linep >= line+sizeof(line)-2)
      linep = line + sizeof(line) - 2;
    linep[0] = ch;
    linep[1] = '\0';
  } while (*linep != '\r' && *linep != '\n' && linep != line + sizeof(line) - 1);

  if (0 == (milliSecs % 5000)) {
    usbPuts(SPRINT("HELLO %10lu %lu %d '%s' %hu %d/%d %d,%d,%d", milliSecs, csCnt, linep?linep-line:0, line, btnMask, 
                                                        deciMilli, msIn,
                                                        ledCnt[0].p1, ledCnt[0].p2, (milliSecs % (ledCnt[0].p1 * 2))));
  }
  csCnt = 0;
  if (ch < 0)
    return;

  *linep = '\0';
  linep = NULL;

  if (0 == strcasecmp_P(line, PSTR("init")))
    initPins();
  else if (0 == strncasecmp_P(line, PSTR("led"), 3) && line[3] >= '0' && line[3] <= '3' && line[4] == '=')
    handleLedCmd(line[3]-'0', line+5);
  else if (0 == strncasecmp_P(line, PSTR("scroll="), 7) && line[7] >= '0' && line[7] <= '1')
    scrollMode = line[7] == '1';
  else if (0 == strcasecmp_P(line, PSTR("help")) || line[0] == '?' || 0 == line[0])
    usbPuts(SPRINT("use:\r\n  init\r\n  scroll=1|0\r\n  ledN=0|1\r\n  ledN=ms1[,ms2]"));
  else {
    usbPuts(SPRINT("ERROR '%s'", line));
    *line = '\0';
    return;
  }
  usbPuts(SPRINT("OK '%s'", line));
  *line = '\0';
}

// vim: set sw=2 sts=2 et:
