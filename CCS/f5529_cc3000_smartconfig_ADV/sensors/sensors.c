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

const char sensorNames[10][11] = {
                                "adc0",
                                "adc1",
                                "adc2",
                                "adc3",
                                "adc4",
                                "p36",
                                "p37",
                                "p38",
                                "p39",
                                "p40"
                                };

void setupSensors(void)
{
	// ADC Ch.	-> Port (LP pin)	// GPIO Ch.
	//--------------------------	//--------------------------
	// A0 			-> P6.0 (23)	// P36 			-> P1.3 (36)
	// A1 			-> P6.1 (24)	// P37 			-> P1.4 (37)
	// A2 			-> P6.2 (25)	// P38 			-> P1.5 (38)
	// A3 			-> P6.3 (26)	// P39 			-> P2.4 (39)
	// A4 			-> P6.4 (27)	// P40 			-> P2.5 (40)

	// Configure ADC settings
	  volatile unsigned int i;
	  WDTCTL = WDTPW+WDTHOLD;                   // Stop watchdog timer
	  P6SEL |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
	  	  	  	  	  	  	  	  	  	  	  	// Enable A/D channel A0,1,2,3,4
	  REFCTL0 &= ~REFMSTR;                      // Reset REFMSTR to hand over control to
												// ADC12_A ref control registers
	  ADC12CTL0 = ADC12ON+ADC12SHT02+ADC12REFON+ADC12REF2_5V;
												// Turn on ADC12, Sampling time
												// On Reference Generator and set to
												// 2.5V
	  ADC12CTL1 = ADC12SHP;                     // Use sampling timer
	  ADC12MCTL0 = ADC12SREF_1;                 // Vr+=Vref+ and Vr-=AVss

	// Configure DIO settings
	  P1DIR |= (BIT3 + BIT4 + BIT5); 			//as inputs
	  P2DIR |= (BIT4 + BIT5);					//as inputs
	  P1OUT &= ~(BIT3 + BIT4 + BIT5);
	  P2OUT &= ~(BIT4 + BIT5);

    // Allow for settling delay
    busyWait(2);
}

int getSensorResult(unsigned char sensorNum)
{
  int dio_status;
  int adcExpire = 0;
  ADC12IFG = 0;
  ADC12CTL0 &= ~ADC12ENC;
  ADC12CTL0 &= ~ADC12SC;

  switch (sensorNum) {
    case A0:
      ADC12MCTL0 = ADC12INCH_0;     // A0 ADC input select; Vref=AVCC
      break;
    case A1:
      ADC12MCTL0 = ADC12INCH_1;     // A1 ADC input select; Vref=AVCC
      break;
    case A2:
      ADC12MCTL0 = ADC12INCH_2;     // A2 ADC input select; Vref=AVCC
      break;
    case A3:
      ADC12MCTL0 = ADC12INCH_3;     // A4 ADC input select; Vref=AVCC
      break;
    case A4:
      ADC12MCTL0 = ADC12INCH_4;     // A5 ADC input select; Vref=AVCC
      break;
    case P36:
      dio_status = (P1IN & BIT3);
      break;
   case P37:
      dio_status = (P1IN & BIT4);
      break;
    case P38:
      dio_status = (P1IN & BIT5);
      break;
    case P39:
      dio_status = (P2IN & BIT4);
      break;
    case P40:
      dio_status = (P2IN & BIT5);
      break;
    default:
      break;
  }

  if ((sensorNum==A0) || (sensorNum==A1) ||(sensorNum==A2) || (sensorNum==A3) || (sensorNum==A4))
  {
	ADC12CTL0 |= ADC12SC + ADC12ENC;                   // Start conversion
	while ((!(ADC12IFG & BIT0)) && (adcExpire<10000))
		{
			adcExpire++;
		}
	//__no_operation();                       // SET BREAKPOINT HERE
	return ADC12MEM0;
	//return (ADC12MEM0 + sensorNum); //FACTORY ONLY
  }
  else
  {
  return dio_status;
  //return (dio_status + sensorNum); //FACTORY ONLY
  }
}
