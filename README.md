# GeoFox - An Arduino + RFM22B based Amateur Radio Fox transmitter.

This repository contains the codebase for the GeoFoxes that have been travelling around the Adelaide area since early Jan 2013.

## Hardware
- Arduino compatible device, running from a 3.3v supply (same as the RFM22B)
- RFM22B Radio Module, wired up as per RF22 library documentation.
- One Button, between pin 3 and ground, with a pullup to 3.3V.
- Optional: LEDs on pins A2 and A3, battery voltage tap (100k/10k) on A7

## Software
- Modified version of the RF22 lib provided. Exposes some private functions I call to set some register parameters.
- Make sure to change the frequency and callsigns in the code!

## TODO
- Add passphrase checking for the remote activation code.
- Add support for powering up a GPS and getting a fix when battery voltage gets low.