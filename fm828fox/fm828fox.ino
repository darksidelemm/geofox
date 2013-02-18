/*
	FM828-Based Morse Beacon
	
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

#define PTT_PIN     3
#define TONE_PIN    6
#define TONE_FREQ   1000

#define BEACON_TIME 20 // Seconds
#define SILENCE_TIME 10

void setup(){
    pinMode(PTT_PIN, OUTPUT);
    pinMode(TONE_PIN, OUTPUT);
    
    digitalWrite(PTT_PIN, HIGH); // Active Low PTT
}

void loop(){
    // Beacon for X seconds.
    ptt_on();
    for(int i = 0; i<BEACON_TIME; i++){
        tone(TONE_PIN,TONE_FREQ,1000);
    }
    ptt_off();
    
    delay(1000);
    
    // Send Morse Ident.
    ptt_on();
    delay(500);
    morse_tx_string("DE VK5VCO 2M FOX");
    delay(500);
    ptt_off();
    
    // Silence
    for(int i = 0; i<SILENCE_TIME; i++){
        delay(1000);
    }

}
    
void ptt_on(){ digitalWrite(PTT_PIN, LOW); }
void ptt_off(){ digitalWrite(PTT_PIN, HIGH); }
