/*****************************************************************************
*
*  board.c - board functions
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
#include "wlan.h" 
#include "evnt_handler.h"    // callback function declaration
#include "nvmem.h"
#include "socket.h"
#include "netapp.h"
#include "board.h"
#include "common.h"
#include "cc3000.h"
#include "sensors.h"
#include "utils.h"

extern unsigned char * ptrFtcAtStartup;
extern volatile unsigned long SendmDNSAdvertisment;

#define LED1_PORT_DIR          P1DIR
#define LED1_PORT_OUT          P1OUT
#define LED2_PORT_DIR          P4DIR
#define LED2_PORT_OUT          P4OUT

//How to init clock: DCO or Crystal select
#define DCO_CLOCK
#ifdef DCO_CLOCK
	#define F_CPU	25000000L	//demo is configured for 25MHz CPU frequency only
	void initClocks(void);
#endif
#ifdef CRYSTAL_CLOCK
	#define XT1_XT2_PORT_SEL       P5SEL
	#define XT1_ENABLE             (BIT4 + BIT5)
	#define XT2_ENABLE             (BIT2 + BIT3)
	#define XT1HFOFFG              0
	#define PMM_STATUS_ERROR       1
	#define PMM_STATUS_OK          0
#endif

//*****************************************************************************
//
//! board_init
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Initialize the board's interfaces
//
//*****************************************************************************
void board_init()
{
  // Stop WDT
  WDTCTL = WDTPW + WDTHOLD;

  // Init GPIO's
  pio_init();

  // Setup sensors hooked up to the board (if any)
  setupSensors();

  #ifndef EN_ADS1118
  // Start CC3000 State Machine
  resetCC3000StateMachine();

  // Initialize Board and CC3000
  initDriver();
  #endif
  // Initialize CC3000 Unsolicited Events Timer
  unsolicicted_events_timer_init();

  // Enable interrupts
  __enable_interrupt();
}

//*****************************************************************************
//
//! pio_init
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Initialize the board's I/O
//
//*****************************************************************************    

void pio_init()
{
	#ifdef DCO_CLOCK
		initClocks();
	#endif
	#ifdef CRYSTAL_CLOCK
		// Setup XT1 and XT2
		XT1_XT2_PORT_SEL |= XT1_ENABLE + XT2_ENABLE;
		// Init device clock
		initClk();
	#endif

	initLEDs();

	P2DIR |= 0x04;
	P4DIR = 0xFF;
	P6DIR = 0xFF;
	P6OUT = 0x00;
	P7DIR = 0xFF;
	P7OUT = 0x00;
	WLAN_EN_OUT &= ~WLAN_EN_PIN;
	WLAN_EN_DIR |= WLAN_EN_PIN;

	// Port initialization for SPI operation
	SPI_IRQ_DIR &= ~SPI_IRQ_PIN;
	SPI_SEL |= SPI_CLK + SPI_SOMI + SPI_SIMO;
	SPI_DIR |= SPI_CLK + SPI_SIMO;
	SPI_REN |= SPI_SOMI;                                   // Pull-Ups on RF Interface SOMI
	SPI_OUT |= SPI_SOMI;

	RF_CS_SEL &= ~RF_CS;
	RF_CS_OUT |= RF_CS;
	RF_CS_DIR |= RF_CS;

	// Globally enable interrupts
	__enable_interrupt();
}

//*****************************************************************************
//
//! ReadWlanInterruptPin
//!
//! @param  none
//!
//! @return none
//!
//! @brief  return wlan interrup pin
//
//*****************************************************************************

long ReadWlanInterruptPin(void)
{
	// Return the status of IRQ
	return    (SPI_IRQ_IN & SPI_IRQ_PIN);
}

//*****************************************************************************
//
//! WlanInterruptEnable
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Enable waln IrQ pin
//
//*****************************************************************************

void WlanInterruptEnable()
{
	SPI_IRQ_IES |= SPI_IRQ_PIN;
	SPI_IRQ_IE |= SPI_IRQ_PIN;
}

//*****************************************************************************
//
//! WlanInterruptDisable
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Disable waln IrQ pin
//
//*****************************************************************************

void WlanInterruptDisable()
{
	SPI_IRQ_IE &= ~SPI_IRQ_PIN;
}

//*****************************************************************************
//
//! WriteWlanPin
//!
//! @param  val value to write to wlan pin
//!
//! @return none
//!
//! @brief  write value to wlan pin
//
//*****************************************************************************
void WriteWlanPin(uint8_t trueFalse)
{	
	if(trueFalse)
	{
		WLAN_EN_OUT |= WLAN_EN_PIN;                 // RF_EN_PIN high
	}
	else
	{
		WLAN_EN_OUT &= ~WLAN_EN_PIN;                // RF_EN_PIN low
	}
}

//*****************************************************************************
//
//! unsolicicted_events_timer_init
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  The function initializes the unsolicited events timer handler
//
//*****************************************************************************
void unsolicicted_events_timer_init(void)
{
	TA1CCTL0 &= ~CCIE; 
	TA1CTL |= MC_0;
	
	// Configure the timer for each 500 milli to handle un-solicited events
	TA1CCR0 = 0x4000;
	
	// run the timer from ACLCK, and enable interrupt of Timer A
	TA1CTL |= (TASSEL_1 + MC_1 + TACLR);
	
	TA1CCTL0 |= CCIE;
}

//*****************************************************************************
//
//! mDNS_packet_trigger_timer_enable
//!
//!  \param  None
//!
//!  \return none
//!
//!  \brief  Enable the Timer A0 to trigger mDNS packet
//
//*****************************************************************************
void mDNS_packet_trigger_timer_enable(void)
{
  TA0CCTL0 &= ~CCIE;                         // TACCR0 interrupt enabled
  TA0CTL |= MC_0;
  // The timer clock is ~10KHz. Dividing it by 8 and than by 8 gives us clock of 156.25 Hz
  TA0EX0 = TAIDEX_7;
  // We want to wakeup each 30 sec which is  0x1248 @ 156.25 Hz
  TA0CCR0 = 0x1248;

  // run the timer from ACLCK, and enable interrupt of Timer A0
  TA0CTL |= (TASSEL_1 + MC_1 + TACLR  + ID_3);

  TA0CCTL0 |= CCIE;
}

//*****************************************************************************
//
//! unsolicicted_events_timer_init
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  The function initializes a CC3000 device and triggers it to start
//!          operation
//
//*****************************************************************************
void unsolicicted_events_timer_disable(void)
{
	TA1CCTL0 &= ~CCIE; 
	TA1CTL |= MC_0;
}

#ifdef DCO_CLOCK
void initClocks(void)
{
#if defined(__MSP430_HAS_UCS__)
     PMMCTL0_H = PMMPW_H;             // open PMM
	 SVSMLCTL &= ~SVSMLRRL_7;         // reset
	 PMMCTL0_L = PMMCOREV_0;          //

	 PMMIFG &= ~(SVSMLDLYIFG|SVMLVLRIFG|SVMLIFG);  // clear flags
	 SVSMLCTL = (SVSMLCTL & ~SVSMLRRL_7) | SVSMLRRL_1;
	 while ((PMMIFG & SVSMLDLYIFG) == 0); // wait till settled
	 while ((PMMIFG & SVMLIFG) == 0); // wait for flag
	 PMMCTL0_L = (PMMCTL0_L & ~PMMCOREV_3) | PMMCOREV_1; // set VCore for lext Speed
	 while ((PMMIFG & SVMLVLRIFG) == 0);  // wait till level reached

	 PMMIFG &= ~(SVSMLDLYIFG|SVMLVLRIFG|SVMLIFG);  // clear flags
	 SVSMLCTL = (SVSMLCTL & ~SVSMLRRL_7) | SVSMLRRL_2;
	 while ((PMMIFG & SVSMLDLYIFG) == 0); // wait till settled
	 while ((PMMIFG & SVMLIFG) == 0); // wait for flag
	 PMMCTL0_L = (PMMCTL0_L & ~PMMCOREV_3) | PMMCOREV_2; // set VCore for lext Speed
	 while ((PMMIFG & SVMLVLRIFG) == 0);  // wait till level reached

	 PMMIFG &= ~(SVSMLDLYIFG|SVMLVLRIFG|SVMLIFG);  // clear flags
	 SVSMLCTL = (SVSMLCTL & ~SVSMLRRL_7) | SVSMLRRL_3;
	 while ((PMMIFG & SVSMLDLYIFG) == 0); // wait till settled
	 while ((PMMIFG & SVMLIFG) == 0); // wait for flag
	 PMMCTL0_L = (PMMCTL0_L & ~PMMCOREV_3) | PMMCOREV_3; // set VCore for lext Speed
	 while ((PMMIFG & SVMLVLRIFG) == 0);  // wait till level reached

     PMMCTL0_H = 0;                   // lock PMM

     UCSCTL0 = 0;                     // set lowest Frequency
#if F_CPU >= 25000000L
     UCSCTL1 = DCORSEL_6;             //Range 6
     UCSCTL2 = 0x1176;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLK|SELS__DCOCLK;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#elif F_CPU >= 24000000L
     UCSCTL1 = DCORSEL_6;             //Range 6
     UCSCTL2 = 0x116D;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLK|SELS__DCOCLK;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#elif F_CPU >= 16000000L
     UCSCTL1 = DCORSEL_6;             //Range 6
     UCSCTL2 = 0x11E7;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLKDIV|SELS__DCOCLKDIV;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#elif F_CPU >= 12000000L
     UCSCTL1 = DCORSEL_6;             //Range 6
     UCSCTL2 = 0x116D;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLKDIV|SELS__DCOCLKDIV;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#elif F_CPU >= 8000000L
     UCSCTL1 = DCORSEL_5;             //Range 5
     UCSCTL2 = 0x10F3;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLKDIV|SELS__DCOCLKDIV;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#elif F_CPU >= 1000000L
     UCSCTL1 = DCORSEL_2;             //Range 2
     UCSCTL2 = 0x101D;                //Loop Control Setting
	 UCSCTL3 = SELREF__REFOCLK;       //REFO for FLL
	 UCSCTL4 = SELA__REFOCLK|SELM__DCOCLKDIV|SELS__DCOCLKDIV;  //Select clock sources
	 UCSCTL7 &= ~(0x07);               //Clear Fault flags
#else
        //#warning No Suitable Frequency found!
#endif
#endif // __MSP430_HAS_UCS__
}
#endif

#ifdef CRYSTAL_CLOCK
//*****************************************************************************
//
//! initClk
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  Init the device with 16 MHz DCOCLCK.
//
//*****************************************************************************
void initClk(void)
{	
	// Set Vcore to accomodate for max. allowed system speed
	SetVCore(3);
	// Use 32.768kHz XTAL as reference
	LFXT_Start(XT1DRIVE_0);
	
	// Set system clock to max (25MHz)
	Init_FLL_Settle(25000, 762);
	SFRIFG1 = 0;
	SFRIE1 |= OFIE;
}

//*****************************************************************************
//
//! LFXT_Start
//!
//! @param  none
//!
//! @return none
//!
//! @brief
//
//*****************************************************************************
void LFXT_Start(uint16_t xtdrive)
{
  // If the drive setting is not already set to maximum
  // Set it to max for LFXT startup
  if ((UCSCTL6 & XT1DRIVE_3)!= XT1DRIVE_3) { 
    UCSCTL6_L |= XT1DRIVE1_L + XT1DRIVE0_L; // Highest drive setting for XT1startup
  }
	
  while (SFRIFG1 & OFIFG) {   // Check OFIFG fault flag
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG); // Clear OSC flaut Flags fault flags
    SFRIFG1 &= ~OFIFG;        // Clear OFIFG fault flag
  }
  
  UCSCTL6 = (UCSCTL6 & ~(XT1DRIVE_3)) | (xtdrive); // set requested Drive mode
}

void Init_FLL_Settle(uint16_t fsystem, uint16_t ratio)
{
  volatile uint16_t x = ratio * 32;       
	
  Init_FLL(fsystem, ratio);
  
  while (x--) {
		__delay_cycles(30); 
  }
}

void Init_FLL(uint16_t fsystem, uint16_t ratio)
{
  uint16_t d, dco_div_bits;
  uint16_t mode = 0;
	
  // Save actual state of FLL loop control, then disable it. This is needed to
  // prevent the FLL from acting as we are making fundamental modifications to
  // the clock setup.
  uint16_t srRegisterState = __get_SR_register() & SCG0;
  __bic_SR_register(SCG0);  
  
  d = ratio;
  dco_div_bits = FLLD__2;        // Have at least a divider of 2
  
  if (fsystem > 16000) {
    d >>= 1 ;
    mode = 1;
  }
  else {
    fsystem <<= 1;               // fsystem = fsystem * 2
  }
	
  while (d > 512) {
    dco_div_bits = dco_div_bits + FLLD0;  // Set next higher div level
    d >>= 1;
  }
	
  UCSCTL0 = 0x0000;              // Set DCO to lowest Tap
	
  UCSCTL2 &= ~(0x03FF);          // Reset FN bits
  UCSCTL2 = dco_div_bits | (d - 1);
	
  if (fsystem <= 630)            //           fsystem < 0.63MHz
		UCSCTL1 = DCORSEL_0;
  else if (fsystem <  1250)      // 0.63MHz < fsystem < 1.25MHz
		UCSCTL1 = DCORSEL_1;
  else if (fsystem <  2500)      // 1.25MHz < fsystem <  2.5MHz
		UCSCTL1 = DCORSEL_2;
  else if (fsystem <  5000)      // 2.5MHz  < fsystem <    5MHz
		UCSCTL1 = DCORSEL_3;
  else if (fsystem <  10000)     // 5MHz    < fsystem <   10MHz
		UCSCTL1 = DCORSEL_4;
  else if (fsystem <  20000)     // 10MHz   < fsystem <   20MHz
		UCSCTL1 = DCORSEL_5;
  else if (fsystem <  40000)     // 20MHz   < fsystem <   40MHz
		UCSCTL1 = DCORSEL_6;
  else
		UCSCTL1 = DCORSEL_7;
	
  while (SFRIFG1 & OFIFG) {                               // Check OFIFG fault flag
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG);     // Clear OSC flaut Flags
    SFRIFG1 &= ~OFIFG;                                    // Clear OFIFG fault flag
  }
	
  if (mode == 1) {                              		  // fsystem > 16000
    SELECT_MCLK_SMCLK(SELM__DCOCLK + SELS__DCOCLK);       // Select DCOCLK
  }
  else {
    SELECT_MCLK_SMCLK(SELM__DCOCLKDIV + SELS__DCOCLKDIV); // Select DCODIVCLK
  }
  
  __bis_SR_register(srRegisterState);	                  // Restore previous SCG0
}

/*******************************************************************************
* \brief   Increase or decrease Vcore levels by one level
*
* \param level     Set levels to Vcores that need to be increased/decreased
* \return status   Success/failure
******************************************************************************/
uint16_t SetVCore(uint8_t level)
{
  uint16_t actlevel;
  uint16_t status = 0;
  
  level &= PMMCOREV_3;                       // Set Mask for Max. level
  actlevel = (PMMCTL0 & PMMCOREV_3);         // Get actual VCore
	// step by step increase or decrease
  while (((level != actlevel) && (status == 0)) || (level < actlevel)) {
    if (level > actlevel) {
      status = SetVCoreUp(++actlevel);
    }
    else {
      status = SetVCoreDown(--actlevel);
    }
  }
  
  return status;
}

/*******************************************************************************
* \brief   Increase Vcore by one level
*
* \param level     Level to which Vcore needs to be increased
* \return status   Success/failure
******************************************************************************/
static uint16_t SetVCoreUp(uint8_t level)
{
  uint16_t PMMRIE_backup, SVSMHCTL_backup, SVSMLCTL_backup;

  // The code flow for increasing the Vcore has been altered to work around
  // the erratum FLASH37.
  // Please refer to the Errata sheet to know if a specific device is affected
  // DO NOT ALTER THIS FUNCTION

  // Open PMM registers for write access
  PMMCTL0_H = 0xA5;

  // Disable dedicated Interrupts
  // Backup all registers
  PMMRIE_backup = PMMRIE;
  PMMRIE &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE |
							SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
  SVSMHCTL_backup = SVSMHCTL;
  SVSMLCTL_backup = SVSMLCTL;

  // Clear flags
  PMMIFG = 0;

  // Set SVM highside to new level and check if a VCore increase is possible
  SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * level);

  // Wait until SVM highside is settled
  while ((PMMIFG & SVSMHDLYIFG) == 0);

  // Clear flag
  PMMIFG &= ~SVSMHDLYIFG;

  // Check if a VCore increase is possible
  if ((PMMIFG & SVMHIFG) == SVMHIFG) {      // -> Vcc is too low for a Vcore increase
  	// recover the previous settings
  	PMMIFG &= ~SVSMHDLYIFG;
  	SVSMHCTL = SVSMHCTL_backup;

  	// Wait until SVM highside is settled
  	while ((PMMIFG & SVSMHDLYIFG) == 0);

  	// Clear all Flags
  	PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

  	PMMRIE = PMMRIE_backup;                 // Restore PMM interrupt enable register
  	PMMCTL0_H = 0x00;                       // Lock PMM registers for write access
  	return PMM_STATUS_ERROR;                // return: voltage not set
  }

  // Set also SVS highside to new level
  // Vcc is high enough for a Vcore increase
  SVSMHCTL |= (SVSHRVL0 * level);

  // Wait until SVM highside is settled
  while ((PMMIFG & SVSMHDLYIFG) == 0);

  // Clear flag
  PMMIFG &= ~SVSMHDLYIFG;

  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;

  // Set SVM, SVS low side to new level
  SVSMLCTL = SVMLE | (SVSMLRRL0 * level) | SVSLE | (SVSLRVL0 * level);

  // Wait until SVM, SVS low side is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);

  // Clear flag
  PMMIFG &= ~SVSMLDLYIFG;
  // SVS, SVM core and high side are now set to protect for the new core level

  // Restore Low side settings
  // Clear all other bits _except_ level settings
  SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

  // Clear level settings in the backup register,keep all other bits
  SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

  // Restore low-side SVS monitor settings
  SVSMLCTL |= SVSMLCTL_backup;

  // Restore High side settings
  // Clear all other bits except level settings
  SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

  // Clear level settings in the backup register,keep all other bits
  SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

  // Restore backup
  SVSMHCTL |= SVSMHCTL_backup;

  // Wait until high side, low side settled
  while (((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0));

  // Clear all Flags
  PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

  PMMRIE = PMMRIE_backup;                   // Restore PMM interrupt enable register
  PMMCTL0_H = 0x00;                         // Lock PMM registers for write access

  return PMM_STATUS_OK;
}

/*******************************************************************************
* \brief  Decrease Vcore by one level
*
* \param  level    Level to which Vcore needs to be decreased
* \return status   Success/failure
******************************************************************************/
static uint16_t SetVCoreDown(uint8_t level)
{
  uint16_t PMMRIE_backup, SVSMHCTL_backup, SVSMLCTL_backup;

  // The code flow for decreasing the Vcore has been altered to work around
  // the erratum FLASH37.
  // Please refer to the Errata sheet to know if a specific device is affected
  // DO NOT ALTER THIS FUNCTION

  // Open PMM registers for write access
  PMMCTL0_H = 0xA5;

  // Disable dedicated Interrupts
  // Backup all registers
  PMMRIE_backup = PMMRIE;
  PMMRIE &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE |
							SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
  SVSMHCTL_backup = SVSMHCTL;
  SVSMLCTL_backup = SVSMLCTL;

  // Clear flags
  PMMIFG &= ~(SVMHIFG | SVSMHDLYIFG | SVMLIFG | SVSMLDLYIFG);

  // Set SVM, SVS high & low side to new settings in normal mode
  SVSMHCTL = SVMHE | (SVSMHRRL0 * level) | SVSHE | (SVSHRVL0 * level);
  SVSMLCTL = SVMLE | (SVSMLRRL0 * level) | SVSLE | (SVSLRVL0 * level);

  // Wait until SVM high side and SVM low side is settled
  while ((PMMIFG & SVSMHDLYIFG) == 0 || (PMMIFG & SVSMLDLYIFG) == 0);

  // Clear flags
  PMMIFG &= ~(SVSMHDLYIFG + SVSMLDLYIFG);
  // SVS, SVM core and high side are now set to protect for the new core level

  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;

  // Restore Low side settings
  // Clear all other bits _except_ level settings
  SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

  // Clear level settings in the backup register,keep all other bits
  SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

  // Restore low-side SVS monitor settings
  SVSMLCTL |= SVSMLCTL_backup;

  // Restore High side settings
  // Clear all other bits except level settings
  SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

  // Clear level settings in the backup register, keep all other bits
  SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

  // Restore backup
  SVSMHCTL |= SVSMHCTL_backup;

  // Wait until high side, low side settled
  while (((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0));

  // Clear all Flags
  PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

  PMMRIE = PMMRIE_backup;                   // Restore PMM interrupt enable register
  PMMCTL0_H = 0x00;                         // Lock PMM registers for write access
  return PMM_STATUS_OK;		                // Return: OK
}
#endif

//*****************************************************************************
//
//! StartDebounceTimer
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Starts timer that handles switch debouncing
//
//*****************************************************************************
void StartDebounceTimer()
{
	// default delay = 0
	// Debounce time = 1500* 1/8000 = ~200ms
	TB0CCTL0 = CCIE;                          // TACCR0 interrupt enabled
	TB0CCR0 = 3000;
	TB0CTL = TBSSEL_1 + MC_1;                 // SMCLK, continuous mode
}

//*****************************************************************************
//
//! StopDebounceTimer
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Stops timer that handles switch debouncing
//
//*****************************************************************************
void StopDebounceTimer()
{
	// TACCR0 interrupt enabled
	TB0CCTL0 &= ~CCIE;
}

//*****************************************************************************
//
//! initLEDs
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Initializes LED Ports and Pins
//
//*****************************************************************************
void initLEDs()
{
	LED1_PORT_OUT &= ~(BIT0 );
	LED1_PORT_DIR |= BIT0 ;
	LED2_PORT_OUT &= ~(BIT7);
	LED2_PORT_DIR |= BIT7;
}

//*****************************************************************************
//
//!  turnLedOn
//!
//! @param  ledNum is the LED Number
//!
//! @return none
//!
//! @brief  Turns a specific LED on
//
//*****************************************************************************
void turnLedOn(char ledNum)
{
	switch(ledNum)
	{
	case LED1:
		LED1_PORT_OUT |= (BIT0);
		break;
	case LED2:
		LED2_PORT_OUT |= (BIT7);
		break;

	}
}

//*****************************************************************************
//
//! turnLedOff
//!
//! @param  ledNum is the LED Number
//!
//! @return none
//!
//! @brief  Turns a specific LED Off
//
//*****************************************************************************
void turnLedOff(char ledNum)
{
	switch(ledNum)
	{
	case LED1:
		LED1_PORT_OUT &= ~(BIT0);
		break;
	case LED2:
		LED2_PORT_OUT &= ~(BIT7);
		break;

	}
}

//*****************************************************************************
//
//! toggleLed
//!
//! @param  ledNum is the LED Number
//!
//! @return none
//!
//! @brief  Toggles a board LED
//
//*****************************************************************************
void toggleLed(char ledNum)
{
	switch(ledNum)
	{
	case LED1:
		LED1_PORT_OUT ^= (BIT0);
		break;
	case LED2:
		LED2_PORT_OUT ^= (BIT7);
		break;

	}
}

//*****************************************************************************
//
//! \brief  set Smart Config flag when S2 was pressed
//!
//! \param  none
//!
//! \return none
//!
//
//*****************************************************************************
void SetFTCflag()
{
   *ptrFtcAtStartup = SMART_CONFIG_SET;                              //  set Smart Config flag
}

//*****************************************************************************
//
//! DissableSwitch
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Dissable S2 switch interrupt
//
//*****************************************************************************
void DissableSwitch()
{
	  // disable switch interrupt
	  P2IFG &= ~BIT1;                // Clear P2.1 IFG
	  P2IE &= ~BIT1;               	 // P2.1 interrupt disabled
	  P2IFG &= ~BIT1;                // Clear P2.1 IFG
	  P2IFG = 0;
}

//*****************************************************************************
//
//! RestoreSwitch
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Restore S2 switch interrupt
//
//*****************************************************************************
void RestoreSwitch()
{
	  // Restore Switch Interrupt
	  P2IFG &= ~BIT1;                 // Clear P2.1 IFG
	  P2IE |= BIT1;                   // P2.1 interrupt enabled
	  P2IFG &= ~BIT1;                 // Clear P2.1 IFG
}

//*****************************************************************************
//
//! \brief  Indication if the switch is still pressed
//!
//! \param  none
//!
//! \return none
//!
//
//*****************************************************************************
long switchIsPressed()
{
 if(!(P2IN & BIT1))
 {
	 return 0;	//SmartConfig Button Pressed
 }
   return 1;
}

//*****************************************************************************
//
//! \brief  Restarts the MSP430
//!
//! Restarts the MSP430 completely. One must be careful
//!
//! \return never
//!
//
//*****************************************************************************
void restartMSP430()
{
 PMMCTL0 |= PMMSWPOR;

 // This function will never exit since it forces a complete
 // restart of the MSP430.
}


// Timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void)
{
	//Set flag to send mDNS Advertisement
	SendmDNSAdvertisment  = 1;
}

//Catch interrupt vectors that are not initialized.
#ifdef __CCS__
	#pragma vector=WDT_VECTOR
	__interrupt void Trap_ISR(void)
	{
	  while(1);
	}
	#pragma vector=TIMER0_A0_VECTOR
	__interrupt void Trap5_ISR(void)
	{
		_nop();//while(1); //FACTORY ONLY
	}
	#pragma vector=TIMER0_A1_VECTOR
	__interrupt void Trap4_ISR(void)
	{
	  while(1);
	}
	#pragma vector=TIMER1_A1_VECTOR
	__interrupt void Trap3_ISR(void)
	{
			while(1);
	}
	#pragma vector=TIMER0_B1_VECTOR
	__interrupt void Trap13_ISR(void)
	{
		while(1);
	}
	#pragma vector=ADC12_VECTOR
	__interrupt void Trap7_ISR(void)
	{
	  while(1);
	}
	#pragma vector=COMP_B_VECTOR
	__interrupt void Trap8_ISR(void)
	{
	  while(1);
	}
	#pragma vector=USB_UBM_VECTOR
	__interrupt void Trap9_ISR(void)
	{
	  while(1);
	}
	#pragma vector=UNMI_VECTOR
	__interrupt void Trap10_ISR(void)
	{
	  while(1);
	}
	#pragma vector=DMA_VECTOR
	__interrupt void Trap11_ISR(void)
	{
	  while(1);
	}
	#pragma vector=RTC_VECTOR
	__interrupt void Trap12_ISR(void)
	{
	  while(1);
	}
	#pragma vector=SYSNMI_VECTOR
	__interrupt void Trap14_ISR(void)
	{
	  while(1);
	}
	#pragma vector=USCI_A0_VECTOR
	__interrupt void Trap15_ISR(void)
	{
	  while(1);
	}
	#pragma vector=USCI_B0_VECTOR
	__interrupt void Trap16_ISR(void)
	{
	  while(1);
	}
	#pragma vector=USCI_B1_VECTOR
	__interrupt void Trap17_ISR(void)
	{
	  while(1);
	}
#endif
