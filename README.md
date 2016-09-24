# ESPFlasher

An open source and cross platform firmware programmer for ESP8266 chip.

## Features

- Write an image to flash.
- Read SPI flash content.
- Download an image to RAM and execute.
- Dump arbitrary memory to disk.
- Read arbitrary memory location.
- Write to arbitrary memory location.
- Run application code in flash.
- Dump headers from an application image.
- Create an application image from multiples binary files.
- Create an application image from ELF file.
- Read MAC address from ROM.
- Read SPI flash manufacturer and device ID.
- Perform Chip Erase on SPI flash.
- Import/Export images list to text file.

## Supported reset modes

Mode       | Description
-----------|-------------
None       | No DTR/RTS manipulation.
Auto       | DTR controls RST via a capacitor, RTS pulls down GPIO0.
CK         | RTS pulls down RST, DTR pulls down GPIO0.
Wifio      | DTR controls RST via a capacitor, TxD controls GPIO0 via a diode (or PNP).
Nodemcu    | DTR and RTS control GPIO0 and RST via NPN transistors.

## Dependencies

ESPFlasher is created with [Qt 5](http://www.qt.io/) and depends on [Poppler Qt5](http://poppler.freedesktop.org/) for barcode PDF generation and printing.

## About

ESPFlasher source code is released under GPLv2.
