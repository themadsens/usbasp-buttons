# usbasp-buttons
Buttons and blinkenlights on a usbasp

Put the seven available GPIO's on the usbasp to use for pushbuttons and LED's via an
[avrcdc](http://www.recursion.jp/prose/avrcdc/) serial port.

Buttons report on the tty as `BUTTON<1..3>=<0|1>`<br/>
LED's are numbered 0..3

__Help text for the tty device__
```
use:
  init
  scroll=1|0
  ledN=0|1
  ledN=ms1[,ms2]
```

__TODO:__
Use libusb.<br/>
Basically, turn the usbasp into a self-flashable [digispark](http://digistump.com/products/1) type device, flashable via [micronucleus](https://github.com/micronucleus/micronucleus). Only the atmega8 on the usbasp has real bootloader support.

