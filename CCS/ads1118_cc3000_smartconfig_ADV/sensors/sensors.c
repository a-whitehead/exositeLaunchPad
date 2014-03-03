/*****************************************************************************
*
*  sensors.c - board sensor functions
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
*****************************************************************************/

#include <msp430.h>
#include "sensors.h"
#include "utils.h"
#include "ADS1118.h"
#include "LCD_driver.h"
#include "exosite.h"
#include "board.h"

const char sensorNames[10][11] = {
									"tmpc",
									"na1",
									"na2",
									"na3",
									"na4",
									"na5",
									"na6",
									"na7",
									"na8",
									"na9"
                                };

int tempValue; //CONV_
int Act_temp;	// Actual temperature
int Act_temp_D;	// Actual temperature
int radioStatus = 0;
int exoinit = 0;
extern int sensorValue[10];
extern int exoTempThr; //temperature update from Exosite
extern unsigned long exoTimer; //temperature update from Exosite
unsigned long exoTimerHH = 0;
unsigned long exoTimerMM = 0;
unsigned long exoTimerSS = 0;
unsigned long time = 0;	// current time, a Continuous number seconds
unsigned int set_time;	// temporary for setting time
unsigned int Thr_temp;	// Threshold temperature
unsigned int set_temp;	// temporary for setting Threshold temperature
unsigned int num=0;	// temporary for setting Threshold temperature
volatile unsigned char Thr_state;	// state for threshold temperature setting state machine.
volatile unsigned char time_state;	// state for time setting state machine.
volatile unsigned int  flag = 0;		//global flag.
/* flag is used to transmit parameter between interrupt service function and main function.
* the flag will be changed by ISR in InterruptVectors_init.c     ...\grace\InterruptVectors_init.c
*
* Bit0, Launchpad S2 is pushed
* Bit1, indicates temp threshold value has changed is to be updated. VOID-->SW1 on BoosterPack is pushed
* Bit2, indicates count/timer value has changed is to be updated. VOID-->SW2 on BoosterPack is pushed
* Bit3, 1 second timer interrupt
* Bit4, timer for ADC interrupts
* Bit5, ADC input flag, 0 is internal temperature sensor, 1 is AIN0-AIN1
* Bit6, make an inversion every half a second
* Bit7, half a second interrupt
* Bit8, for Fahrenheit display
* Bit9, ADC channel flag, 0 for channel 0, 1 for channel 1.
* BitA, interrupt for live temperature update
* BitB, returning to regular operation after live temperature interrupt
* BitC, print a status message once in looping condition
* BitD, display Temperature Threshold and Timer Deadline
* BitE, enable or disable Exosite read
* BitF, ADC run once on start-up
*/

void setupSensors(void)
{
	//P1DIR |= BIT1; //ADS1118 C or F degree display
	P1REN |= BIT1;
	P1OUT |= BIT1;
	P1IE |= BIT1;
	P1IES |= BIT1;
	P1IFG &= ~BIT1;

	P2DIR |= BIT3 + BIT6 + BIT7; //CS:6=LCD, 7=ADS1118 // 3=RS = Register Select Signal. RS=0: instruction; RS=1: data
	P2OUT |= BIT6 + BIT7;

	P4REN |= BIT2 + BIT1; //SW1 SW2 of ADS1118 pull-ups
	P4OUT |= BIT2 + BIT1;
	P4DIR = 0xF9; //0xFF; CONV_ for C or F degree select

	P8DIR = BIT1; 	//LCD_RST Active LOW Reset Signal
	P8OUT |= BIT1;
}

int getSensorResult(unsigned char sensorNum)
{
 switch (sensorNum) {
	case NA1:
		tempValue = 0;
	  break;
	case TMPC:
	  if (!(flag & BITF))
	  {
		tempValue = ADC_display();     // A1 ADC input select; Vref=AVCC
		flag |= BITF;
	  }
		tempValue = ADC_display();     // A1 ADC input select; Vref=AVCC
	  break;
	default:
	  break;
  }
  return tempValue;
}

//*****************************************************************************
//
//!  ads1118Main
//!
//!  \param  None
//!
//!  \return None
//!
//!  \brief  configure or handle temperature value sensed and LCD
//
//*****************************************************************************
void ads1118Main()
{
	//Read temperature and update display
	if(radioStatus == 0)
	  { 										//Check if already init
		  WLAN_EN_OUT &= ~WLAN_EN_PIN;          // RF_EN_PIN low to put CC3000 in shut-down mode
		  init_spi_ads1118(); 					//config SPI for ADS1118BP
		  unsigned char sensorCount = 0;
		  char strRead[6];
		  for (sensorCount = 0; sensorCount < NA1; sensorCount++) 			//SENSOR_END
				{
				  sensorValue[sensorCount] = getSensorResult(sensorCount);	//get the sensor reading
				  itoa(sensorValue[sensorCount], strRead, 10);
					  sendString("\t");
					  sendString(strRead);
				}
		  sendString("\r\n");

		  if (flag & BITD)
		  {
			  ads1118Extra(); 					//configure or handle threshold, timer, channel select, and degree unit display
		  }

		  if (!(flag & BITA))
		  {
			  if (!(flag & BITB))
			  {
				  ads1118Extra(); 				//configure or handle threshold, timer, channel select, and degree unit display
			  }
			  resetCC3000StateMachine();		// Start CC3000 State Machine
			  initDriver();
			  if (exoinit == 0){
			  if (!Exosite_Init("exosite", "cc3000wifismartconfig", IF_WIFI, 0))
				{
				  show_status();
				  while(1);
				}
			  }
		  exoinit = 1;
		  radioStatus = 1;
		  }
	  }
}

int ADC_display()
{
	static signed int local_data, far_data;
	signed int temp;
		busyWait(50);
		local_data = ADS_Read(1);	//read local temperature data,and start a new convertion for far-end temperature sensor.

		busyWait(50);
		far_data = ADS_Read(0);		//read far-end temperature,and start a new convertion for local temperature sensor.
		temp = far_data + local_compensation(local_data);	// transform the local_data to compensation codes of far-end.

		temp = ADC_code2temp(temp);	// transform the far-end thermocouple codes to temperature.

		if(flag & BIT8)				// display temperature in Fahrenheit
		{
			Act_temp = temp;
			LCD_delay_Nms(1);
			LCD_display_temp(1,0,Act_temp);
			LCD_display_char(1,6,'C');
		}
		else
		{
			Act_temp = temp;
			Act_temp_D = temp * 9 / 5 +320;
			LCD_delay_Nms(1);
			LCD_display_temp(1,0,Act_temp_D);
			LCD_display_char(1,6,'F');
		}
		return Act_temp;
}

void init_spi_ads1118(void)
{
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCCKPL | UCMSB | UCMST | UCMODE_0 | UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    UCB0BR0 = 100;
    UCB0CTL1 &= ~UCSWRST;
}

/*
 * function name:System_Initial()
 * description: Initialize the system. include I/O, LCD and ADS1118.
 */
void lcd_system_Initial()
{
	Thr_state = 0;  //threshold temperature setting state machine counter
	time_state = 0;	//time setting state machine counter
	Thr_temp = 100; //configure threshold temperature to 100;
	Act_temp = 250;

	LCD_init();						// LCD initial
	LCD_clear();					// LCD clear
	LCD_display_string(0,"Exosite Synced @");
	LCD_display_char(1,5,0xDF);
	LCD_display_time(1,8,time); //LCD_display_time(1,8,time);
}

/*
 * function name:ads1118Extra()
 * description: configure or handle threshold, timer, channel select, and degree unit display
 */
void ads1118Extra()
{
	if (!(flag & BITD))
	{
		if (flag & BIT3)
		{
			time_display();
		}

		if(flag & BIT1)
		{
			Thr_temp = exoTempThr;			// assign threshold temperature
			//Check if threshold exceeds current temperature. If so, buzz.
			if (Act_temp_D/10 >= exoTempThr)
			{
				LCD_display_string(0,"Threshold Reach!");
				//TODO add code to activate buzzer
				_BIS_SR(LPM0_bits + GIE); // Enter LPM3 w/ interrupt
			}
		}

		if(flag & BIT2)	// if SW2 is pushed, and Thr_state = 0, time setting state machine will be changed
		{
			//Check if timer exceeds deadline. If so, buzz. //Non-1 second timer Option
			exoTimerHH = (exoTimer/10000)*3600;
			exoTimerMM = ((exoTimer/100)-((exoTimer/10000)*100))*60;
			exoTimerSS = (exoTimer - ((exoTimer/10000)*10000) - ((exoTimer/100)-((exoTimer/10000)*100))*100);
			exoTimer = exoTimerHH + exoTimerMM + exoTimerSS;
			if (time >= exoTimer)
			{
				LCD_display_string(0,"Timer Expired!  ");
				//TODO add code to activate buzzer
				_BIS_SR(LPM0_bits + GIE); // Enter LPM3 w/o interrupt
			}
		}
		else
		__no_operation();
	}
	else
	{
		//NOTE: Display Temperature Threshold and Timer Deadline
		LCD_display_string(0,"TH:");
		LCD_display_time(0,8,exoTimer);
		LCD_display_number(0,3,Thr_temp);
	}
}

void time_display()
{
	flag &= ~BIT3;						// flag is reset
	if (time_state == 0)
	{

  		if(time >= 432000 || time <= 0)	// if current is more than 24 hours //CONV_ (time >=86400
  		{
  			time = 0;
  		}
  		LCD_display_time(1,8,time);		// display time on LCD
	}
}

/*
 *  Timer0_A2
 *  Initialize MSP430 Timer0_A2 timer
*/
void Timer2_A0_Init(void)
{
	TA2CCR0 = 0x4000;
	TA2CTL = TASSEL_1 + MC_1 + TACLR + TAIE + ID_1;
}

// Timer1_A3 Interrupt Vector (TAIV) handler
#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMER2_A1_ISR(void)
{
  switch(__even_in_range(TA2IV,14))
  {
    case  0: break;                          // No interrupt
    case  2: break;                          // CCR1 not used
    case  4: break;                          // CCR2 not used
    case  6: break;                          // reserved
    case  8: break;                          // reserved
    case 10: break;                          // reserved
    case 12: break;                          // reserved
    case 14: 								 // overflow
    		flag |= BIT7;
    		flag ^= BIT6;
    		if (!(flag & BIT6))
    			flag |= BIT3;
    		time++;
             break;
    default: break;
  }
}
