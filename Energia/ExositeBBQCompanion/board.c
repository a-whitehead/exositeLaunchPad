#include <msp430.h>
#include "board.h"

#define __MSP430F5529__

extern volatile unsigned int flag;
extern volatile int totalExpire;
extern volatile int total;
// /*
// *  ======== Timer2_A1 Interrupt Service Routine ========
// */
#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMER2_A1_ISR(void)
{
	total--;
}
// /*
// *  ======== Timer0_A3 Interrupt Service Routine ========
// */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void)
{
    /* USER CODE START (section: TIMER0_A0_ISR_HOOK) */
	flag |= BIT7;
	flag ^= BIT6;
	if (!(flag & BIT6))
	flag |= BIT3;
    /* USER CODE END (section: TIMER0_A0_ISR_HOOK) */
	
	totalExpire++;
	if (totalExpire == 120) // set to 60 for ADS1118 only operation
	{
		total--;
		totalExpire = 0;
	}
}
