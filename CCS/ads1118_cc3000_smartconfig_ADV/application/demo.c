/****************************************************************
* BBQ Companion BoosterPack Combination Exosite Demo for
* MSP430F5529 LaunchPad, CC3000 BoosterPack, and ADS1118 BoosterPack.
* Rev. 01
* 29.Jan.2014
*
* DESCRIPTION:
* This program is intended to work with the hardware combination
* of an MSP430F5529 LaunchPad, CC3000 BoosterPack, and ADS1118
* BoosterPack. The demonstration enables the full feature availability
* of the ADS1118 BoosterPack's type-K thermocouple and LCD to
* accompany the barebones Exosite demo.
*
* During the write command, the ADS1118 collected temperature data is
* transmitted to the Exosite service. The ADS1118 collects the temperature
* data before the CC3000 is activated.
*
* During the read command, the temperature threshold, count-up timer
* deadline, and degree unit indicator values are received by the BBQ
* Companion. A local alert will indicate that the threshold or deadline
* has expired or exceeded.
*
* The ADS1118 BoosterPack display is updated after every Exosite
* Read/Write cycle, thus the temperature value displayed does not
* reflect the real-time value.
*
* AUTHOR:   Akeem Whitehead		DATE: 29 JAN 2014
*
* CHANGES:
* REV.    DATE        WHO    DETAIL
*
* BUILT WITH:
* Code Composer Studio Core Edition Version: 5.5
* (C) Copyright Texas Instruments, 2014. All rights reserved.
*****************************************************************/
/****************************************************************
*
*  demo.c - CC3000 Main Demo Application
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************/
#include <msp430.h>
#include "wlan.h" 
#include "evnt_handler.h"    // callback function declaration
#include "nvmem.h"
#include "socket.h"
#include "common.h"
#include "netapp.h"
#include "cc3000.h"
#include "sensors.h"
#include "board.h"
#include "string.h"
#include "utils.h"
#include "spi.h"
#include "uart.h"
#include "../exosite/exosite.h"
#include "common_adv.h"

// ADS1118BP Custom variables and functions
int sensorValue[10];
int exoTempThr;
int exoTempThrPrev;
extern int radioStatus;
extern volatile unsigned int  flag;
unsigned int exoTimerPrev;
unsigned long exoTimer;

extern void Timer2_A0_Init();
extern void init_spi_ads1118(void);
extern void lcd_system_Initial();
extern void ads1118Main();
extern void LCD_display_string(unsigned char L, char *ptr);

// Custom boot function
#if defined(EN_COM_CONFIG) || defined(EN_ADS1118)
int _system_pre_init( void )
{
   WDTCTL = WDTPW + WDTHOLD;
   return 1;
}   // _system_pre_init
#endif

//*****************************************************************************
//
//!  main
//!
//!  \param  None
//!
//!  \return none
//!
//!  \brief   The main loop is executed here
//
//*****************************************************************************
void main(void)
{
  unsigned char loopCount = 0;
  int loop_time = 2000;

  ulCC3000Connected = 0;
  SendmDNSAdvertisment = 0;

  // Initialize hardware and interfaces
  board_init();
  initUart();
  Timer2_A0_Init();

  if (!(flag & BITB))
  	{
	  busyWait(50);
	  sendString("\r\nSystem init : \r\n");
  	}
  else
  {
	  sendString("\r\nSystem Re-init : \r\n");
  }

  if (!(flag & BITB))
  	{
	  init_spi_ads1118(); //config SPI for ADS1118BP
	  lcd_system_Initial(); //config LCD
      //NOTE: Displaying "Exosite Synced @ T:00.0oF 0000hrs"
  	}

  #ifdef EN_COM_CONFIG
  	  initNetworkInitRead(passMAC, PTRpassMAC, sizeof(passMAC));				// Read MAC from Flash due to dynamic network configuration
  	  initNetworkInitRead(passDEVNAME, PTRpassDEVNAME, sizeof(passDEVNAME));	// Read DevServName from Flash due to dynamic network configuration
  	  initNetworkInitRead(passMODNAME, PTRpassMODNAME, sizeof(passMODNAME));	// Read ModelName from Flash due to dynamic network configuration
  	  memcpy(DevServname, passDEVNAME, sizeof(passDEVNAME));

  	  // Initialize and confirm network credentials during first iteration
  	  // If no Device Name name is provided, do not time during dynamic configuration prompt
  	  if (passDEVNAME[0] != 255)
  	  {
  	  	  configFlag |= BIT6;
		  initNetworkConfig();
		  configFlag &= ~BIT6;
		  expireCount=0;
  	  }
  	  else
  	  {
  		initNetworkConfig();
  	  }
  	  // Must initialize one time for MAC address prepare..
  	  if (!Exosite_Init("exosite", passMODNAME, IF_WIFI, 0))
  #endif
  #ifndef EN_COM_CONFIG
	  if (!Exosite_Init("exosite", "cc3000wifismartconfig", IF_WIFI, 0))	// HARDCODED - Model Name
  #endif
	  {
		show_status();
		while(1);
	  }

  // Main Loop
  while (1)
  {
	configFlag |= BITB;
	ads1118Main();
	flag &= ~BITB;

	// Perform Smart Config if button pressed in current run or if flag set from previous MSP430 Run.
	if(runSmartConfig == 1 || *ptrFtcAtStartup == SMART_CONFIG_SET)
    {
      // Clear flag
      ClearFTCflag();
      unsetCC3000MachineState(CC3000_ASSOC);

      // Start the Smart Config Process
      sendString("\tStarting Smart Config...\r\n");
      StartSmartConfig();
      runSmartConfig = 0;
    }
    // If connectivity is good, run the primary functionality
    if(checkWiFiConnected())
    {
      memset(exo_buffer, 0, sizeof(exo_buffer));
      char * pbuf = exo_buffer;
      //unsolicicted_events_timer_disable(); //FACTORY ONLY
      configFlag &= ~BIT9;
      expireCount=0;
      configFlag |= BIT8;

      if (0 == cloud_status)
      { //check to see if we have a valid connection
        loop_time = 2000;

        loopCount = 1;

// START EXOSITE READ
// PROGRAMMER NOTE: To disable Exosite Read command, block comment from "// START EXOSITE READ" to "// END EXOSITE READ"
        while (loopCount++ <= (WRITE_INTERVAL+1))
        {
            sendString("== Exosite Read==\r\n");
              if (Exosite_Read("dgr_ctrl", pbuf, EXO_BUFFER_SIZE))
				{
                  if (!strncmp(pbuf, "0", 1))
                  {
                	  flag ^= BIT8; //LCD Display in Fahrenheit
                  }
                  else if (!strncmp(pbuf, "1", 1))
                  {
                	  flag |= BIT8; //LCD Display in Celsius
                  }
				}
              if (Exosite_Read("thr_ctrl", pbuf, EXO_BUFFER_SIZE))
				{
				exoTempThr = atoi(pbuf);
				  if(exoTempThr != exoTempThrPrev)
				  {
					flag |= BIT1;
					sendString("\tTemperature Threshold updated.\r\n");
				  }
				exoTempThrPrev = exoTempThr;
				}
			  if (Exosite_Read("timer_ctrl", pbuf, EXO_BUFFER_SIZE))
				{
				  exoTimer = atol(pbuf); //NOTE: limited to 65536 or 6hr55min36sec
				  if (exoTimer != exoTimerPrev)
				  {
					flag |= BIT2;
					sendString("\tCountdown Timer updated.\r\n");
				  }
				exoTimerPrev = exoTimer;
				}
			  else
				{
					if (EXO_STATUS_NOAUTH == Exosite_StatusCode())
					{
						turnLedOff(LED2);
						// Activate device again
						cloud_status = Exosite_Activate();
						sendString("\tExosite Read fail!\r\n");
					}
				}

          hci_unsolicited_event_handler();
          unsolicicted_events_timer_init();
          busyWait(loop_time);	//delay before looping again
        }
// END EXOSITE READ
// START EXOSITE WRITE
// PROGRAMMER NOTE: To disable Exosite Write command, block comment from "START EXOSITE WRITE" to "END EXOSITE WRITE"
        unsolicicted_events_timer_init();
    	if (EXO_STATUS_NOAUTH != Exosite_StatusCode())
    	{
    	  sendString("== Exosite Write==\r\n\t");
    	  unsigned char sensorCount = 0;
          int value;
          char strRead[6]; //largest value of an int in ascii is 5 + null terminate
          for (sensorCount = 0; sensorCount < SENSOR_END; sensorCount++)
          {
        	  value = sensorValue[sensorCount];
			  itoa(value, strRead, 10);                 //convert to a string
			  unsolicicted_events_timer_init();
			  //for each reading / data source (alias), we need to build the string "alias=value" (must be URL encoded)
			  //this is all just an iteration of, for example, Exosite_Write("mydata=hello_world",18);
			  memcpy(pbuf,&sensorNames[sensorCount][0],strlen(&sensorNames[sensorCount][0]));  //copy alias name into buffer
			  pbuf += strlen(&sensorNames[sensorCount][0]);
			  *pbuf++ = 0x3d;                           //put an '=' into buffer
			  memcpy(pbuf,strRead, strlen(strRead));    //copy value into buffer
			  pbuf += strlen(strRead);
			  *pbuf++ = 0x26;                           //put an '&' into buffer, the '&' ties successive alias=val pairs together
          }
          pbuf--;										//back out the last '&'
		  sendString(exo_buffer);
		  sendString("\r\n");
          Exosite_Write(exo_buffer,(pbuf - exo_buffer - 1));    //write all sensor values to the cloud
          configFlag &= ~BIT8;
          expireCount=0;

          if (EXO_STATUS_OK == Exosite_StatusCode())
          {
        	  sendString("\tWrite SUCCESS!\r\n");
        	  radioStatus = 0;
          }
          else
		  {
			  sendString("\tWrite FAIL!\r\n");
			  radioStatus = 0;
			  show_status();
		  }
    	}
      }
// END EXOSITE WRITE
      else {
          //don't have a good connection yet - keep retrying to authenticate
          sendString("== Exosite Activate==\r\n");
          sendString("\tBad connection. Retry authentication. Delay 30s...\r\n");
          cloud_status = Exosite_Activate();
          if (0 != cloud_status) loop_time = 30000; //delay 30 seconds before retrying...
      }
        unsolicicted_events_timer_init();
      }
      // TODO - make this a sleep instead of busy wait
      busyWait(loop_time);	//delay before looping again
    }
} //main

//*****************************************************************************
//
//!  checkWiFiConnected
//!
//!  \param  None
//!
//!  \return TRUE if connected, FALSE if not
//!
//!  \brief  Checks to see that WiFi is still connected.  If not associated
//!          with an AP for 5 consecutive retries, it will reset the board.
//
//*****************************************************************************
unsigned char
checkWiFiConnected(void)
{
  unsigned char ipInfoFlagSet = 0;
  if(!(currentCC3000State() & CC3000_ASSOC)) //try to associate with an Access Point
  {
    // Check whether Smart Config was run previously. If it was, we use it to connect to an access point. Otherwise, we connect to the default.
    if((isFTCSet() == 0)&&(ConnectUsingSmartConfig==0)&&(*SmartConfigProfilestored != SMART_CONFIG_SET))
    {
      // Smart Config not set, check whether we have an SSID from the assoc terminal command. If not, use fixed SSID.
    	sendString("== ConnectUsingSSID==\r\n");
    	#ifdef EN_COM_CONFIG
    		ConnectUsingSSID((char *)passSSID, (unsigned char *)passPASS, (unsigned)atol(passSECU));
		#endif
		#ifndef EN_COM_CONFIG
    		ConnectUsingSSID(SSID, (unsigned char*)PASSPHRASE, SECURITY); //Enable to bypass manual network configuration
		#endif
    }
    unsolicicted_events_timer_init();
    // Wait until connection is finished
    sendString("== Wait until connection is finished==\r\n");

    int sc_button_wait = 0;
    int sc_button_wait_clr = 0;
    sendString("\t(also checking Smart Config button)\r\n\t");
    configFlag &= ~BIT8;
    expireCount=0;
    configFlag |= BIT7;
    while (!(currentCC3000State() & CC3000_ASSOC))
    {
      __delay_cycles(100);
      // Handle any un-solicited event if required - the function will get triggered few times in a second
      hci_unsolicited_event_handler();
      if(sc_button_wait == 20000)
      {
    	  sendString(".");
    	  if (sc_button_wait_clr == 10)
    	  {
    		  sendString("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    		  sc_button_wait = 0;
    		  sc_button_wait_clr = 0;
    	  }
    	  else
    	  {
    		  sc_button_wait_clr++;
    		  sc_button_wait = 0;
    	  }
      }
      else{
    	  sc_button_wait++;
      }
      // Check if user pressed button to do Smart Config
      if(runSmartConfig == 1)
          break;
    }
  }
  sendString("\r\n");

  // Handle un-solicited events - will be triggered few times per second
  hci_unsolicited_event_handler();
  // Check if we are in a connected state.  If so, set flags and LED
  if(currentCC3000State() & CC3000_IP_ALLOC)
  {
    unsolicicted_events_timer_disable(); // Turn our timer off since isr-driven routines set LEDs too...
    if (obtainIpInfoFlag == FALSE)
    {
      sendString("== CC3000_IP_ALLOC_IND==\r\n");
      obtainIpInfoFlag = TRUE;             // Set flag so we don't constantly turn the LED on
      //CC3000_IP_ALLOC_IND = TRUE; //FACTORY ONLY
      ipInfoFlagSet = 1;
      unsolicicted_events_timer_init();
    }
    if (obtainIpInfoFlag == TRUE)
    {
      //If Smart Config was performed, we need to send complete notification to the configure (Smart Phone App)
      if (ConnectUsingSmartConfig==1)
      {
    	sendString("\tConnected using Smart Config!\r\n");
    	mdnsAdvertiser(1,DevServname, sizeof(DevServname));
        ConnectUsingSmartConfig = 0;
        *SmartConfigProfilestored = SMART_CONFIG_SET;
      }
      //Start mDNS timer in order to send mDNS Advertisement every 30 seconds
      mDNS_packet_trigger_timer_enable();
      unsolicicted_events_timer_init();
    }
    if( ipInfoFlagSet == 1)
    {
      // Initialize an Exosite connection
      sendString("== Exosite Activate ==\r\n");
      configFlag &= ~BIT7;
      configFlag=0;
      configFlag |= BIT9;
      cloud_status = Exosite_Activate();
      ipInfoFlagSet = 0;

    }
    sendString("\tYES! Connected to Network & Exosite!\r\n");
    return TRUE;
  }
  sendString("\tNO! Not connected to Network & Exosite!\r\n");
  return FALSE;
} //checkWifiConnected

/*****************************************************************************
*
*  show_status
*
*  \param  None
*
*  \return None
*
*  \brief  Shows the status message on the LCD display
*
*****************************************************************************/
void show_status(void)
{
  int code = Exosite_StatusCode();
  switch (code)
  {
    case EXO_STATUS_BAD_TCP:
    	_nop();
      break;
    case EXO_STATUS_BAD_UUID:
    	_nop();
      break;
    case EXO_STATUS_BAD_VENDOR:
    	_nop();
      break;
    case EXO_STATUS_BAD_MODEL:
    	_nop();
      break;
    case EXO_STATUS_BAD_INIT:
    	_nop();
      break;
    case EXO_STATUS_BAD_SN:
    	_nop();
      break;
    case EXO_STATUS_CONFLICT:
    	_nop();
      break;
    case EXO_STATUS_BAD_CIK:
    	_nop();
      break;
    case EXO_STATUS_NOAUTH:
    	_nop();
      break;
  }
  return;
} //show_status

/*****************************************************************************
*
*  Interrupts
*
*****************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  switch(__even_in_range(P1IV,P1IV_P1IFG1))
  {
    case P1IV_P1IFG0:
      StartDebounceTimer();
      DissableSwitch();		// disable switch interrupt
      break;
// FACTORY ONLY
/*
    case P1IV_P1IFG1: //ADS1118 toggle C vs F degree display
    	busyWait(50);
    	WLAN_EN_PORT_OUT &= ~WLAN_EN_PIN;
    	busyWait(50);
    	sendString("\r\n\tStart Live Temperature Monitoring\r\n");
		flag |= BITA + BITD;
		radioStatus = 0;
		while (!(P1IN & BIT1))
    	{
			ads1118Main();
			// Enable Disable Read Exosite
			if (!(P4IN & BIT1) && !(P4IN & BIT2) )
			{
				flag ^= BITE;
				busyWait(50);        //delay before looping again
				if (flag & BITE)
				{
					sendString("\tRead is Disabled\r\n");
				}
				else
				{
					sendString("\tRead is Enabled\r\n");
				}
			}
			// Degree Display Select: Fahrenheit or Celsius
			else if (!(P4IN & BIT2))
			{
				flag ^= BIT8;
				busyWait(50);        //delay before looping again
			}
			// Temperature Channel Select: CH0 or CH1
			else if (!(P4IN & BIT1))
			{
				flag ^= BIT9;
				busyWait(50);        //delay before looping again
			}
			else{}
    	}
    	sendString("\tEnd Live Temperature Monitoring\r\n");
    	LCD_display_string(0,"Exosite Synced @");
		flag &= ~(BITA + BITD);
		flag |= BITB;
		P1IFG = 0;
		main();
    break;
*/

    default:
      break;
  }
    P1IFG = 0;
}

// Timer1 A0 interrupt service routine
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
{
  // Check whether button is still pressed. If so, Smart Config should be performed.
  if((!(switchIsPressed())) & (configFlag & BITB))
  {
	// Button is still pressed, so Smart Config should be done
    runSmartConfig = 1;
  	configFlag &= ~BIT7;
  	expireCount=0;
    if(currentCC3000State() & CC3000_IP_ALLOC)
    {
      // Since accept and the server is blocking, we will indicate that Smart Config should be run at startup.
      SetFTCflag();
      // Clear Smart Config profile stored flag until connection established. To use the default SSID for connection, press S1 and then the reset button.
      *SmartConfigProfilestored = 0xFF;
      restartMSP430();
     }
  }
  // Restore Switch Interrupt
  RestoreSwitch();
  StopDebounceTimer();

  // Increment and check expiration count
  if (configFlag & BIT6){ 		//increment at dynamic update prompt
	  expireCount++;
	  if (expireCount == 10){ 	//force 'n'after 5 seconds
		  configFlag &= ~BIT6;
		  expireCount=0;
		  sendByte('n');
		  configChoice='n';
		  configFlag |= BIT1;
	  }
  }
  if (configFlag & BIT7){ 		//increment at waiting for smartconfig button
	  expireCount++;
	  if (expireCount == 120){ 	//enter low power mode 3 after 60 seconds
		  configFlag &= ~BIT7;
		  expireCount=0;
		  //__bis_SR_register(LPM3_bits + GIE); //FACTORY ONLY
	  }
  }

  if (configFlag & BIT8){ 		//increment within exosite r/w routine while network connected
	  expireCount++;
	  if (expireCount == 240){	//restart msp after 120 seconds
		  configFlag &= ~BIT8;
		  expireCount=0;
		  restartMSP430();
	  }
  }
  if (configFlag & BIT9){ 		//increment at "Exosite Activate"
	  expireCount++;
	  if (expireCount == 240){ 	//restart msp after 120 seconds
		  configFlag &= ~BIT9;
		  expireCount=0;
		  restartMSP430();
	  }
  }
}
