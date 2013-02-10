# GeoFox - An Arduino + RFM22B based Amateur Radio Fox transmitter.

This repository contains the codebase for the GeoFoxes that have been travelling around the Adelaide area since early Jan 2013.

These foxes operate on the 70cm Amateur Radio band, running at about 100mW. The standard transmission sequence (trivially edited) is:
- 20s constant carrier
- 30WPM Morse ident
- 50 baud 8N1 RTTY Telemetry, containing battery voltage and temperature
- 3 second listen time, for commands.

The fox can start up in 'quiet' mode by holding down a button on power up. The fox is then switched into 'active' mode when it hears a 500 baud GMSK packet containing '$1'. An active fox can also be switched into quiet mode if it received a '$0' packet.

Pressing the button while the fox is active will cause it to switch to a different frequency (specified in the code), and transmit the '$1' activation packet a number of times. This packet has a narrow (<3KHz) bandwidth, and can be received and replayed by a SSB receiver for longer distance activation. This feature can also be used for multi-leg hunts, where one fox activates the next in a sequence.

## Required Hardware
- Arduino compatible device, running from a 3.3v supply (same as the RFM22B)
- [RFM22B](http://ava.upuaut.net/store/index.php?route=product/product&path=71_63&product_id=65) Radio Module, wired up as per [RF22 library documentation](http://www.open.com.au/mikem/arduino/RF22/).
- One momentary button, between pin 3 and ground, with a pullup to 3.3V.
- Optional: LEDs on pins A2 and A3, battery voltage tap (100k/10k) on A7

## Required Software
- Modified version of the [RF22](http://www.open.com.au/mikem/arduino/RF22/) lib provided. Exposes some private functions I call to set some register parameters.
- Make sure to change the frequency and callsigns in the code!

## TODO
- Add passphrase checking for the remote activation code.
- Add support for powering up a GPS and getting a fix when battery voltage gets low.