This repository contains [I2C][i2c] address scanners for the
ATmega328p AVR and the GigaDevice GD32VF103CBT6
RISC-V MCU platforms.

How to use it: Hookup your I2C devices to a - say - Arduino Pro
Mini 3.3 V (8 MHz) or a Longan Nano MCU board, upload a program
from this repository and connect to the UART to watch the
scanning progress.

Use case: For checking if the MCU's I2C unit, connected devices
and I2C connections and the bus works in principle at certain
speeds and pull-up resistor values. Or for simply verifying the address
of a certain I2C device.

See also [my I2C blog post](https://gms.tf/i2c-scanner.html) for an extended discussion.

## Build Instructions

The simplest way to build and upload a program is to use
[PlatformIO][pio].

For the Atmega328p version:

```
cd atmega328p
pio project init --board pro8MHzatmega328 --ide vim
make upload
```

GD32VG103 (Longan Nano) version:


```
cd gd32vf103
pio project init --board gd32vf103c_longan_nano --ide vim
make upload
```

## Example Output

```
$ picocom --baud 38400 /dev/ttyUSB0
Scanning I2C bus at 100 kHz ...
Found device on address: 0x60 (96)
Found device on address: 0x68 (104)
Scanning I2C bus at 400 kHz ...
Found device on address: 0x60 (96)
Found device on address: 0x68 (104)
done

[.. scan cycle repeats after 30 s or so ..]
```

## ATmega328p Notes

This ATmega328p I2C scanner is written in C and only uses [avr-libc][alibc]
headers, i.e. it doesn't use any Arduino libraries/APIs.

Thus, it's also an illustration of direct interfacing with AVR's
I2C (a.k.a. 2-wire, Two Wire Interface (TWI)) unit. See also the
[ATmega328p datasheet][avr] (Section 22) for details.  Arguably,
interfacing with the MCU directly is just a little bit more
verbose than using the object oriented abstraction the Arduino
framework provides (cf. `Wire.h`). Perhaps the ATmega328p I2C
register interface already is relatively high level, e.g. one
doesn't need to poll for the I2C bus being free for sending a
START, the MCU implements polling and START transmission as one
atomic operation and one just has to poll (or wait) for START being
completed.

The AVR version targets devices with 8 MHz CPU frequency, for
other configurations one has to adjust the I2C clock frequency
setup functions.


## GD32VF103 Notes

The GD32VF103 version is also written in C and uses the Nuclei
SDK, which (unlike the avr-libc) already provides an API for I2C.
Thus, this version is a bit shorter. However, the setup code is
more complex than the AVR MCU one. The [GD32VF103 User Manual][gd]
describes the I2C unit on a register and procedural level, such
that one has to map that description to the Nuclei API. (reading
the source code helps) However, the manual arguably is less
accessible and complete than - say - the ATmega 328p one,
certainly it leaves some setup and usage aspects open.

Thus, one has to experiment a bit when creating an I2C
program from scratch.


## Related Work

- Wolfgang Ewald's [I2C scanner](https://wolles-elektronikkiste.de/en/i2c-scanner) that uses the Arduino `Wire.h` API (2020)
- Nick Gammon's [I2C scanner](http://www.gammon.com.au/i2c#reply6) that also uses Arduino's `Wire.h` API (2011)
- Andreas Müller's [I2C C++ class](https://github.com/MuellerA/LonganNanoTest/tree/master/I2c/src) for the Longan Nano and an example for interfacing with an LCD

[i2c]: https://en.wikipedia.org/wiki/I%C2%B2C
[alibc]: https://www.nongnu.org/avr-libc/user-manual/modules.html
[pio]: https://github.com/platformio/platformio-core/
[gd]: http://www.gd32mcu.com/download/down/document_id/222/path_type/1
[avr]: https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf
