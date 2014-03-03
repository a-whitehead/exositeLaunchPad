//*****************************************************************************
//
// wifi_read_write_string - A Simple read/write to the Exosite Cloud API
//                          for the LaunchPad's CC3000 WiFi BoosterPack
//
// Copyright (c) 2013 Exosite LLC.  All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:

//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of Exosite LLC nor the names of its contributors may
//    be used to endorse or promote products derived from this software 
//    without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

// Modified: NAME            DATE         NOTES
//           A. Whitehead    14FEB2014    - Only works on MSP-EXP430F5529LP.
//                                        - Not enough ROM/RAM on MSP-EXP430G2LP.
//                                        - To be ported to Tiva C LaunchPad.

#include "Energia.h"
#include <SPI.h>
#include <WiFi.h>
#include <Exosite.h>

void setup();
void loop();
void printWifiStatus();
void sw1_interrupt();
void sw2_interrupt();

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
String cikData = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // <-- Fill in your CIK here! (https://portals.exosite.com -> Add Device)
char ssid[] = "insertSSID";       // your network name (SSID)
char pass[] = "insertPassword";   // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

// Use these variables to customize what datasources are read and written to.
String readString0 = "led1_ctrl";
String readString1 = "led2_ctrl";
String writeString0 = "adc0=";
String writeString1 = "adc1=";
String writeString2 = "adc2=";
String writeString3 = "adc3=";
String writeString4 = "adc4=";
String writeString5 = "p36=";
String writeString6 = "p37=";
String writeString7 = "p38=";
String writeString8 = "p39=";
String writeString9 = "p40=";
String writeString10 = "sw1=";
String writeString11 = "sw2=";
String returnString;

const int adc0 = A0;
const int adc1 = A1;
const int adc2 = A2;
const int adc3 = A3;
const int adc4 = A4;
const int button1Pin = PUSH2;
const int button2Pin = PUSH2;
char sw1_stat = '0';
char sw2_stat = '0';

/*==============================================================================
* End of Configuration Variables
*=============================================================================*/

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(50, 62, 217, 1); // numeric IP for Energia.nu (no DNS)
//char server[] = "m2.exosite.com";    // name address for Energia.nu (using DNS)

// Initialize the WiFi client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
//WiFiClient client;

//int status = WL_IDLE_STATUS;
class WiFiClient client;
Exosite exosite(cikData, &client);

/*==============================================================================
* setup
*
* Energia setup function.
*=============================================================================*/
void setup(){  

  pinMode(adc0, INPUT);
  pinMode(adc1, INPUT);
  pinMode(adc2, INPUT);
  pinMode(adc3, INPUT);
  pinMode(adc4, INPUT);

  pinMode(36, INPUT_PULLUP);
  pinMode(37, INPUT_PULLUP);
  pinMode(38, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(40, INPUT_PULLUP);

  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);

  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  Serial.begin(9600);
  
  // Set communication pins for CC3000
  WiFi.setCSpin(18);  // 18: P2_2 @ F5529, PE_0 @ LM4F/TM4C
  WiFi.setENpin(2);   //  2: P6_5 @ F5529, PB_5 @ LM4F/TM4C
  WiFi.setIRQpin(19); // 19: P2_0 @ F5529, PB_2 @ LM4F/TM4C
  
  delay(200);
  Serial.println("");
  Serial.println("Boot");
  
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
  WiFi.begin(ssid, pass);
  digitalWrite(RED_LED, HIGH);
  Serial.println("Connected to wifi");
  Serial.println("Waiting for DHCP address....");
  // wait for 5 seconds for connection:
  delay(5000);
  printWifiStatus();
}

/*==============================================================================
* loop 
*
* Energia loop function.
*=============================================================================*/
void loop(){
  if ( exosite.writeRead(
		  writeString0+analogRead(adc0)+"&"
		  +writeString1+analogRead(adc1)+"&"
		  +writeString2+analogRead(adc2)+"&"
		  +writeString3+analogRead(adc3)+"&"
		  +writeString4+analogRead(adc4)+"&"
		  +writeString5+digitalRead(36)+"&"
		  +writeString6+digitalRead(37)+"&"
		  +writeString7+digitalRead(38)+"&"
		  +writeString8+digitalRead(39)+"&"
		  +writeString9+digitalRead(40)+"&"
		  +writeString10+sw1_stat+"&"
		  +writeString11+sw2_stat, readString0+"&"+readString1, returnString)){
	digitalWrite(GREEN_LED, HIGH);
    Serial.println("OK");
    //Serial.println(returnString);
  }else{
	digitalWrite(GREEN_LED, LOW);
    Serial.println("Error");
  }
    
  // UNCOMMENT TO ENABLE INTERRUPT DETECTION OF SW1 & SW2 BUTTON PRESS WITHIN A 5 SECOND WINDOW
    //  Serial.println("Switch Interrupts Enabled"); 
    //  attachInterrupt(button1Pin, sw1_interrupt, FALLING);
    //  attachInterrupt(button2Pin, sw2_interrupt, FALLING);
    //  delay(5000);
    //  detachInterrupt(button1Pin);
    //  detachInterrupt(button2Pin); 
    //  Serial.println("Switch Interrupts Disabled"); 
  // END UNCOMMENT OF SW1 & SW2 INTERRUPT
  
  delay(5000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

/*==============================================================================
* isr
*
* Interrupts
*=============================================================================*/
void sw1_interrupt()
{
  detachInterrupt(button1Pin);
  if (sw1_stat == '0'){sw1_stat = '1';}
  else if (sw1_stat == '1'){sw1_stat = '0';}
  delay(1000);
}

void sw2_interrupt()
{
  detachInterrupt(button2Pin);
  if (sw2_stat == '0'){sw2_stat = '1';}
  else if (sw2_stat == '1'){ sw2_stat = '0';}
  delay(1000);
}