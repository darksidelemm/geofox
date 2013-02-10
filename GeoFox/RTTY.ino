/*
	GeoFox - 50 baud 8N1 RTTY
	
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


// RTTY Functions - from RJHARRISON's AVR Code
void rtty_txstring (char * string)
{
	RFM22B_RTTY_Mode();
    rf22.setModeTx();
	/* Simple function to sent a char at a time to 
	** rtty_txbyte function. 
	** NB Each char is one byte (8 Bits)
	*/
	char c;

	c = *string++;
	while ( c != '\0')
	{
		wdt_reset();
		rtty_txbyte (c);
		c = *string++;
	}
	rf22.setModeRx();

}
 
void rtty_txbyte (char c)
{
	/* Simple function to sent each bit of a char to 
	** rtty_txbit function. 
	** NB The bits are sent Least Significant Bit first
	**
	** All chars should be preceded with a 0 and 
	** proceded with a 1. 0 = Start bit; 1 = Stop bit
	**
	*/
	int i;
	rtty_txbit (0); // Start bit
	// Send bits for for char LSB first	
	for (i=0;i<8;i++)
	{
		if (c & 1) rtty_txbit(1); 
			else rtty_txbit(0);	
		c = c >> 1;
	}
	rtty_txbit (1); // Stop bit
    rtty_txbit (1); // Stop bit
}
 
 
// FSK is accomplished by twiddling the frequency offset register. This gives us approx 315Hz shift. 
void rtty_txbit (int bit)
{
		if (bit)
		{
		  // high
               
                  rf22.spiWrite(0x073, 0x03);
		}
		else
		{
		  // low
               
                  rf22.spiWrite(0x073, 0x00);
		}

 		delayMicroseconds(19500); // 50 baud RTTY
}
