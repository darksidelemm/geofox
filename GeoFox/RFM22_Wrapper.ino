/*
	GeoFox - RFM22B Wrapper Functions
	
	Copyright (C) 2013 Mark Jessop <mark.jessop@adelaide.edu.au>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.

*/


// Defined in the main file.
//RF22::ModemConfig GMSK_500bd=  { 0x2b, 0x03, 0xd0, 0xe0, 0x10, 0x62,0x00, 0x05, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x01, 0xA4, 0x2c, 0x23, 0x01 };


void RFM22B_RTTY_Mode(){
	rf22.setFrequency(TX_FREQ);
	rf22.setModeTx();
	rf22.setModemConfig(RF22::UnmodulatedCarrier);
	rf22.spiWrite(0x073, 0x03); // Make sure we're on the high tone when we start
}


void RFM22B_RX_Mode(){
	rf22.setModeIdle(); // For some reason we need to go into an idle mode before switching to RX mode.
	rf22.setModemRegisters(&GMSK_500bd);
    rf22.setPreambleLength(GMSK_PREAMBLE_LEN);
    rf22.spiWrite(0x02A, 0x10); // Set the AFC register to allow +- (0x10 * 625Hz) pull-in (~+-10KHz)
	rf22.setModeRx(); 
	rf22.setFrequency(RX_FREQ);
}

