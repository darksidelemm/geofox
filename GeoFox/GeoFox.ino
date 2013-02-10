/*
	GeoFox
	
	Control Code for a RFM22B + Arduino based radio fox.
	
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

#include <RF22.h>
#include <SPI.h>
#include <util/crc16.h>
#include <avr/wdt.h>


// Pin definitions
#define PWR_LED	    A2
#define	STATUS_LED	A3
#define	BUTTON	    3       // This pin is also HW interrupt 1
#define BUTTON_INTERRUPT    1
#define WIRE_FET    8       // Osiris board specific. Used for triggering balloon cutdowns.
#define VALVE_FET   7       // I might end up using this to power up a GPS.

#define BATT	    7       // Analog input for battery measurement. 
                            // On the Osiris LDO boards, this has a 100k/10k voltage divider.
#define RF22_SLAVESELECT	10
#define RF22_INTERRUPT		0


// Frequency and power settings
// Note: The RFM22B's frequency accuracy is pretty bad, so these frequencies are usually
// figured out empirically.

// Frequencies for this fox
#define TX_FREQ	438.855 
#define	RX_FREQ	438.855

// Frequency of the slave fox, which this fox will activate
#define SLAVE_FREQ  438.880 


#define TX_POWER_HI		RF22_TXPOW_20DBM  // Options are 1,2,5,8,11,14,17,20 dBm
#define TX_POWER_LOW	RF22_TXPOW_11DBM  // Options are 1,2,5,8,11,14,17,20 dBm
#define GMSK_PREAMBLE_LEN  8    // Used for more reliable packet recpetion.

// Beacon settings.
// Transmit sequence is: Carrier, morse ident, RTTY, listen for RX, sleep.
#define BEACON_REPEATS	    1	// Repeat the constant carrier sequence this many times
#define BEACON_ON_TIME	    20	// Carrier on for this many seconds
#define BEACON_OFF_TIME	    0 	// Carrier off for this many seconds.
#define BEACON_SLEEP_TIME   0   // At the end of each sequence, sleep for this many seconds



// Singleton instance of the RFM22B Library 
RF22 rf22(RF22_SLAVESELECT,RF22_INTERRUPT);

// 500 baud GMSK register settings
RF22::ModemConfig GMSK_500bd=  { 0x2b, 0x03, 0xd0, 0xe0, 0x10, 0x62,0x00, 0x05, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x04, 0x19, 0x2c, 0x23, 0x01 };// these values correspond to 500bps tx rate no manchestger OOK - and are the lowest values that are


// Variables & Buffers
char txbuffer [128];
int	rfm_temp;
int8_t ext_temp;
int rssi_floor = 0;
int last_rssi = 0;
char relaymessage[40] = "No uplink received yet";
int batt_mv = 0;
unsigned int count = 0;
unsigned int rx_count = 0;
unsigned int tx_active = 1;

uint8_t slave_fire = 0;
uint8_t slave_has_been_fired = 0;

void setup(){
	// Setup our IO pins
	pinMode(PWR_LED, OUTPUT);
	pinMode(STATUS_LED, OUTPUT);
	pinMode(WIRE_FET, OUTPUT);
	pinMode(VALVE_FET, OUTPUT);
	pinMode(BUTTON, INPUT);
	digitalWrite(PWR_LED, LOW);
	digitalWrite(STATUS_LED, LOW);
	digitalWrite(WIRE_FET, LOW);
	digitalWrite(VALVE_FET, LOW);
	
	// Check the button's status, to see if we should start in quiet mode
	if(digitalRead(BUTTON) == LOW){
	    tx_active = 0;
	    delay(5000);
	}
	
	// Attempt to start the radio. If fail, blink.
	if(!rf22.init()) payload_fail();
	rf22.setTxPower(TX_POWER_HI|0x08);
  	RFM22B_RTTY_Mode();
  	rf22.spiWrite(0x073, 0x00);
  	
  	// Start up the watchdog. This will reset if any of the SPI Comms to the RFM22B fail.
  	wdt_enable(WDTO_8S);
    
    // 30WPM Morse for the idents
	morse_set_wpm(30);
	
	// Attach the button interrupt
	attachInterrupt(BUTTON_INTERRUPT,button_isr, FALLING);
}

void loop(){
	wdt_reset();

	if(tx_active){
		RFM22B_RTTY_Mode();
		rf22.setTxPower(TX_POWER_HI|0x08); // Go back to full power.
		rf22.spiWrite(0x073, 0x00);
		
		// If the button on the fox has been pressed, add an 'alert' sound to the transmission
		// sequence to alert other hunters.
		if(slave_has_been_fired) alert_sound(5);
		
		// Beacon for a while
		for(int i = 0; i<BEACON_REPEATS; i++){
			rf22.setModeTx();
			for(int k = 0; k<BEACON_ON_TIME; k++){
				wdt_reset();
				delay(1000);
				if(slave_fire) signal_fox();
			}
			rf22.setModeRx();
			for(int k = 0; k<BEACON_OFF_TIME; k++){
				wdt_reset();
				delay(1000);
			}
		    delay(1000);
		}
		
		// Check the state of the button between each section of the transmit sequence
		if(slave_fire) signal_fox();
		
		// Read sensors & battery voltages
		rfm_temp = (rf22.temperatureRead( RF22_TSRANGE_M64_64C,0 ) / 2) - 64;
		//batt_mv = (int)(analogRead(BATT) * 3.2);   // Battery Voltage for Boost Converter boards.
		batt_mv = (int)(analogRead(BATT) * 32.82); // Battery Voltage for LDO boards.
		
		// Transmit some Morse
		wdt_reset();
		morse_tx_string("VK5QI FOX");
		wdt_reset();
		delay(1000);
		
		if(slave_fire) signal_fox();
		
		// Transmit some RTTY
		sprintf(txbuffer, "VK5QI FOX %d mV, %d degC, %d ", batt_mv, rfm_temp, count);
		rtty_txstring(txbuffer);
		wdt_reset();
		rtty_txstring(txbuffer);
		
		if(slave_fire) signal_fox();
		
		// Listen for data for 3 seconds
		digitalWrite(PWR_LED, HIGH);
		wdt_reset();
		wait_for_rx(3000);
		digitalWrite(PWR_LED, LOW);
		
		if(slave_fire) signal_fox();
		

	}else{
	    // RX only mode
	    wdt_reset();
		wait_for_rx(5000);
	}
    count++;
}

// Handle a button press. Raises a flag to be handled at the next free point in the transmit sequence.
// Also lights up a LED, which will turn off when the fox activation packet has been handled.
// Button presses will only register if we are in transmit mode.
void button_isr(){
    if(tx_active){
        slave_fire = 1;
        slave_has_been_fired = 1;
        digitalWrite(STATUS_LED, HIGH);
    }
}   

// Transmit an activation packet to a second fox
void signal_fox(){

    // Copied from the RFM22B helper code.
    rf22.setModeIdle(); 
	rf22.setModemRegisters(&GMSK_500bd);
    rf22.setPreambleLength(GMSK_PREAMBLE_LEN);
    rf22.setTxPower(RF22_TXPOW_20DBM|0x08); // Go to full power (100mW) for this
    rf22.spiWrite(0x02A, 0x10); // Set the AFC register to allow +- (0x10 * 625Hz) pull-in (~+-10KHz)
	rf22.setModeRx(); 
	rf22.setFrequency(SLAVE_FREQ);  
	
	// Retransmit 4 times.
	for(int i = 0; i<4; i++){
	    uint8_t data_ack[] = "$1 start";
	    rf22.send(data_ack, sizeof(data_ack));
	    rf22.waitPacketSent();
	    delay(500);
	}
	
	// Reset the flags, turn LED off.
	slave_fire = 0;
	digitalWrite(STATUS_LED, LOW);
	
	// Switch back to our regular transmit power
	rf22.setTxPower(TX_POWER_HI|0x08);
	RFM22B_RTTY_Mode();

}

// Some nasty variable-power blip stuff. Was too hard to DF so I'm not using it.
unsigned int blippower = 0;
void tx_blip(){
	RFM22B_RTTY_Mode();
	switch(blippower%5){
		case 0:
			rf22.setTxPower(RF22_TXPOW_2DBM|0x08);
			break;
		case 1:
			rf22.setTxPower(RF22_TXPOW_14DBM|0x08);
			break;
		case 2:
			rf22.setTxPower(RF22_TXPOW_8DBM|0x08);
			break;
		case 3:
			rf22.setTxPower(RF22_TXPOW_17DBM|0x08);
			break;
		case 4:
			rf22.setTxPower(RF22_TXPOW_5DBM|0x08);
			break;
		default:
			rf22.setTxPower(RF22_TXPOW_11DBM|0x08);
			break;
	}
	rf22.setModeTx();
	delay(50);
	rf22.setModeRx();
	blippower++;
}


// Wait for a RX packet.
// RX packets are really simple, a '$' followed by a command number.
// $0 - Quiet mode
// $1 - Active mode
// TODO: Add passphrase checking for security
int wait_for_rx(uint16_t listen_time){
		RFM22B_RX_Mode();
		delay(400);
		uint8_t buf[RF22_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);
		wdt_reset();
		if (rf22.waitAvailableTimeout(listen_time)){
			wdt_reset();
			last_rssi = ((int)rf22.lastRssi()*51 - 12400)/100;
			if(rf22.recv(buf, &len)){
				rx_count++;
				delay(500); // Wait a bit
				wdt_reset();
				alert_sound(3); // Send an alert to confirm packet reception
				
				// Command decoding
				if(buf[0] == '$'){
					switch(buf[1]){
						case '0':           // Disable fox transmission.
							tx_active = 0;
							break;
						case '1':           // Enable fox transmission.
						    tx_active = 1;
							break;
						default:
							break;
					}
			
			    // The rest of this is from the OSIRIS code, which re-sends the packet comment
			    // in the RTTY packet. I'm not using this for the fox.
			
					// We don't want the command numbers transmitter down, just send the text back
					for (int lc = 3; lc < len; lc += 1){
						if(char(buf[lc]) != ','){ // Try not to break habitat's string parsing.
							relaymessage[lc-3]=char(buf[lc]); 
						}else{
							relaymessage[lc-3] = ' ';
						}
	
					}
					relaymessage[len-3] = 0;
					RFM22B_RTTY_Mode();
					return 1;
				}else{
					// We don't know what this command is, just send the entire thing back.
					for (int lc = 0; lc < len; lc += 1){
						if(char(buf[lc]) != ','){
							relaymessage[lc]=char(buf[lc]);
						}else{
							relaymessage[lc] = ' ';
						}
					}
					relaymessage[len] = 0;
				}
			}
		}
		return 0;
}

// Generate a quick alert sound, audible on a SSB receiver
void alert_sound(int loops){
	RFM22B_RTTY_Mode();
	delay(100);
	for(int i = 0; i<loops; i++){
		rf22.spiWrite(0x073, 0x00);
		delay(250);
		rf22.spiWrite(0x073, 0x02);
		delay(250);
		wdt_reset();
	}
}

// Blink both LEDs alternately and fast, to indicate a failure.
void payload_fail(){
	while(1){
		digitalWrite(PWR_LED, HIGH);
		digitalWrite(STATUS_LED, LOW);
		delay(300);
		digitalWrite(PWR_LED, LOW);
		digitalWrite(STATUS_LED, HIGH);
		delay(300);
		wdt_reset();
	}
}
