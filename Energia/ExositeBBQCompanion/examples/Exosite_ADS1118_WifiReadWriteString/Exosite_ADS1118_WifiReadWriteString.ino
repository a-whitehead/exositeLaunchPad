//*****************************************************************************
// Exosite_ADS1118_WifiReadWriteString.ino
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
// Description:
// This program is intended to work with the hardware combination
// of an MSP430F5529 LaunchPad, CC3000 BoosterPack, and ADS1118
// BoosterPack. The demonstration enables the full feature availability
// of the ADS1118 BoosterPack's type-K thermocouple and LCD to
// accompany the barebones Exosite demo.
// This demo only utilizes the Exosite Write feature, but contains a placeholder for
// the Exosite Read feature by using the exosite.writeRead function. The ADS1118BP
// will capture a temperature value, transmit this value to the Exosite service,
// and update the assoicated Exosite Dashboard.

#include <Exosite.h>
#include <SPI.h>
#include <WiFi.h>
#include <ADS1118.h>
#include <adstemp.h>
#include <board.h>
#include <LCD_driver.h>
#include <mspinit.h>
#include <UART_TxRx.h>

void setup();
void loop();
void printWifiStatus();

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
String readString0 = "dgr_ctrl";
String readString1 = "thr_ctrl";
String readString2 = "timer_ctrl";
String writeString0 = "tmpc=";
String writeString1 = "na0=";
String returnString;
int returnStringAsInt = 100;
char returnStringBuf[50];
char returnStringBufSpc[3];
char dgr_ctrl = 'F';
int timerHH = 00;
int timerMM = 00;
int timerSS = 00;
int timerOld = 0;
int timerNew = 0;
boolean timerFlag = true;

//ADS1118BP local variables
float value;
int result;

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

//int status = WL_IDLE_STATUS;
class WiFiClient client;
Exosite exosite(cikData, &client);

/*==============================================================================
* setup
*
* Energia setup function.
*=============================================================================*/
void setup(){  
  
  delay(100);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Boot");
  digitalWrite(18, HIGH); //DeAssert SPI CS
  delay(100);  
 
  // Initialize the ADS1118 and LCD
  Ads.adsinit();
  Ads.adssettime(2,30,15);
  Ads.LCDstring(0,"TH:");
  Ads.LCDnumber(0,3,100);
  Ads.LCDstring(1,"Temp:        CH1");
  Ads.LCDchar(1,10,0xDF);
  Ads.LCDchar(1,11,dgr_ctrl);
  Ads.adsStarttime();
  
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  value= Ads.adsReadtemp(dgr_ctrl);
  value= Ads.adsReadtemp(dgr_ctrl);

  // Set communication pins for CC3000
  WiFi.setCSpin(18);  // 18: P2_2 @ F5529, PE_0 @ LM4F/TM4C
  WiFi.setENpin(2);   //  2: P6_5 @ F5529, PB_5 @ LM4F/TM4C
  WiFi.setIRQpin(19); // 19: P2_0 @ F5529, PB_2 @ LM4F/TM4C

  // attempt to connect to Wifi network:
  Serial.begin(9600);
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
  if (dgr_ctrl == 'C')
  {
    result = value*10;
  }
  else
  {
    result = ((value-32)*5/9)*10;
  }
  Serial.println(result);  
  if (exosite.writeRead(writeString0+result+"&"+writeString1+"0", 
                        readString2+"&"+readString1+"&"+readString0, 
                        returnString))
  {
    digitalWrite(GREEN_LED, HIGH);
    
    //extract only the numerical data from "dgr_ctrl=x" string
    returnString.toCharArray(returnStringBuf, sizeof(returnStringBuf));
    int j = 0;
    for (int i = 9; i<10; i++)
    { 
      returnStringBufSpc[j] = returnStringBuf[i];
      j++;
    }
    dgr_ctrl = returnStringBufSpc[0];
    
    //extract only the numerical data from "thr_ctrl=xxx" string
    j = 0;
    for (int i = 20; i<24; i++)
    { 
      returnStringBufSpc[j] = returnStringBuf[i];
      j++;
    }
    returnStringAsInt = atoi(returnStringBufSpc);
    
    //extract only the numerical data from timer_ctrl=xxxxxx" string
    memset(returnStringBufSpc,' ',3);
    j = 0;
    int k = 35;
    //Parse HH,MM, and SS from timer string
    for (int i = k+1; i<k+3; i++)
    { 
      returnStringBufSpc[j] = returnStringBuf[i];
      j++;
    }
    timerHH = atoi(returnStringBufSpc);
    j = 0;
    for (int i = k+3; i<k+5; i++)
    { 
      returnStringBufSpc[j] = returnStringBuf[i];
      j++;
    }
    timerMM = atoi(returnStringBufSpc);
    j = 0;
    for (int i = k+5; i<k+7; i++)
    { 
      returnStringBufSpc[j] = returnStringBuf[i];
      j++;
    }
    timerSS = atoi(returnStringBufSpc);
    timerNew = timerHH+timerMM+timerSS;
    if (timerOld != timerNew)
    {
      timerFlag = true; 
    }
    timerOld = timerNew; 
      
  }else{
	digitalWrite(GREEN_LED, LOW);
    Serial.println("Error");
  }
  
  //reinit ADS1118BP SPI
  WiFi.disconnect();
  digitalWrite(RED_LED, LOW);
  digitalWrite(18, HIGH); //DeAssert SPI CS
  digitalWrite(2, LOW);
  delay(1000);  
  Ads.adsreinit();
  Ads.LCDnumber(0,3,returnStringAsInt); //Update temperature threshold
  if (dgr_ctrl == '1')
  {
    dgr_ctrl = 'C';
  }
  else
  {
    dgr_ctrl = 'F';
  }
  Ads.LCDchar(1,11,dgr_ctrl);
  value= Ads.adsReadtemp(dgr_ctrl);
  result= Ads.adsComp(returnStringAsInt,dgr_ctrl);
  if (result==true)
  {
    Serial.println("Temperature Threshold Exceeded!");
  }
  if (timerFlag == true)
  {
    Ads.adssettime(timerHH,timerMM,timerSS);
    timerFlag = false; 
  }
  Ads.adsStarttime();
    
  //reinit CC3000BP SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  digitalWrite(18, LOW); //Assert SPI CS
  digitalWrite(2, HIGH);
  delay(2000);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
  WiFi.begin(ssid, pass);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  Serial.println("Connected to wifi");
  Serial.println("Waiting for DHCP address....");
  // wait for 5 seconds for connection:
  delay(5000);
  printWifiStatus();
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
