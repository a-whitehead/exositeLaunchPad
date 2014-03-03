/*****************************************************************************
*
*  board.h - board definitions
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

#ifndef BOARD_H
#define	BOARD_H

#include <stdint.h>

#define SPI_IRQ_PIN       	BIT0

#define SPI_SIMO        	BIT0
#define SPI_SOMI        	BIT1
#define SPI_CLK         	BIT2
#define RF_CS           	BIT2

// Ports
#define SPI_SEL         	P3SEL
#define SPI_DIR         	P3DIR
#define SPI_OUT         	P3OUT
#define SPI_REN         	P3REN

#define RF_CS_SEL       	P2SEL
#define RF_CS_OUT       	P2OUT
#define RF_CS_DIR       	P2DIR

#define SPI_IRQ_DIR       	P2DIR
#define SPI_IRQ_IN        	P2IN
#define SPI_IRQ_IES       	P2IES
#define SPI_IRQ_IE        	P2IE
#define SPI_IFG_PORT      	P2IFG

#define WLAN_EN_DIR       	P6DIR
#define WLAN_EN_OUT       	P6OUT

#define SPI_CS_PORT_OUT		P2OUT
#define SPI_CS_PORT_DIR    	P2DIR
#define SPI_CS_PORT_SEL    	P2SEL
#define SPI_CS_PIN         	BIT2

#define SPI_IRQ_PORT_DIR   	P2DIR
#define SPI_IRQ_PORT_SEL   	P2SEL

#define WLAN_EN_PORT_OUT   	P6OUT
#define WLAN_EN_PORT_DIR   	P6DIR
#define WLAN_EN_PORT_SEL   	P6SEL
#define WLAN_EN_PIN        	BIT5

#define UART1_PORT   		P2SEL0
#define UART1_PIN    		BIT0
#define UART2_PORT   		P2SEL1
#define UART2_PIN    		BIT1

typedef enum
{
    NO_LED,
    LED1,
    LED2
} ledEnum;

/*  This macro is for use by other macros to form a fully valid C statement.  */
#define st(x)      do { x } while (__LINE__ == -1)

/* Select source for MCLK and SMCLK e.g. SELECT_MCLK_SMCLK(SELM__DCOCLK + SELS__DCOCLK) */
#define SELECT_MCLK_SMCLK(sources) st(UCSCTL4 = (UCSCTL4 & ~(SELM_7 + SELS_7)) | (sources);)

void pio_init();
void StartDebounceTimer();
void StopDebounceTimer();
void initLEDs();
void SetFTCflag();
void ClearFTCflag();
void RestoreSwitch();
long switchIsPressed();
long ReadWlanInterruptPin(void);
void WlanInterruptEnable();
void WlanInterruptDisable();
void WriteWlanPin( unsigned char val );
void unsolicicted_events_timer_init(void);
void unsolicicted_events_timer_disable(void);
void initClk(void);
void DissableSwitch();
void turnLedOn(char ledNum);
void turnLedOff(char ledNum);
void toggleLed(char ledNum);
void mDNS_packet_trigger_timer_enable(void);
void restartMSP430();
void LFXT_Start(uint16_t xtdrive);
void Init_FLL_Settle(uint16_t fsystem, uint16_t ratio);
void Init_FLL(uint16_t fsystem, uint16_t ratio);
uint16_t SetVCore(uint8_t level);
uint16_t SetVCoreUp(uint8_t level);
uint16_t SetVCoreDown(uint8_t level);

#endif
