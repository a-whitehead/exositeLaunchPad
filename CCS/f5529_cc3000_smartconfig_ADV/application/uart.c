/*****************************************************************************
*
*  uart.c  - Board UART functions
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
#include "common.h"
#include "netapp.h"
#include "cc3000.h"
#include "sensors.h"
#include "board.h"
#include "string.h"
#include "uart.h"
#include "terminal.h"

int bytesAvail = 0; //char bytesAvail = 0;
char uartRXBuf[50];
extern char configChoice;
extern int configFlag;

void initUart()
{
	P4SEL = BIT5 + BIT4;                    // P4.4,5 = USCI_A1 TXD/RXD

	UCA1CTL1 |= UCSWRST;                    // **Put state machine in reset**
	UCA1CTL0 = 0x00;
	UCA1CTL1 = UCSSEL__SMCLK + UCSWRST;      // Use SMCLK, keep RESET
	UCA1BR0 = 0x2C;                         // 25MHz/115200= 217.01 =0xD9 (see User's Guide)
	UCA1BR1 = 0x0A;   						// 25MHz/9600= 2604 =0xA2C (see User's Guide)

	UCA1MCTL = UCBRS_3 + UCBRF_0;           // Modulation UCBRSx=3, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

	/* Enable RX Interrupt on UART */
	UCA1IFG &= ~ (UCRXIFG | UCRXIFG);
	UCA1IE |= UCRXIE;
}

//*****************************************************************************
//
//!  \brief Sends a string of characters using the UART module
//!   
//!  \param  msg is pointer to the null terminated string to be sent
//!
//!  \return none
//
//*****************************************************************************
void sendString(char * msg)
{
  unsigned int i = 0;
  for(i = 0; i < strlen(msg); i++)
  {
    while (!(UCA1IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
    UCA1TXBUF = msg[i];
  }	
}

//*****************************************************************************
//
//!  \brief Sends a byte
//!   
//!  \param  b is the byte to be sent through UART
//!
//!  \return none
//
//*****************************************************************************
void sendByte(char b)
{
    while (!(UCA1IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
    UCA1TXBUF = b;
}

//*****************************************************************************
//
//!  \brief UART Module ISR
//!   
//!  In this Interrupt handler we actually run the terminal program. Though
//!  this is typically not recommended because of nested/missed interrupts,
//!  the blocking behavior of the BSD functions make this the best approach.
//!
//!  \param  None
//!
//!  \return none
//
//*****************************************************************************
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{    
    switch(__even_in_range(UCA1IV,0x08))
    {
      case 0: break; // Vector 0 - no interrupt
      case 2:  // Vector 2 - RXIFG
        if(bytesAvail > sizeof(uartRXBuf))
        {
            bytesAvail = 0;
        }

        if (!(UCA1RXBUF==0x0D))// && (configFlag & BIT5)))
		{
        	uartRXBuf[bytesAvail] = UCA1RXBUF;
        	bytesAvail++;
		}
        //runUARTTerminal(); //FACTORY ONLY

        configChoice = UCA1RXBUF; 	//Dynamic manual network config
        configFlag |= BIT1;			//Dynamic configChoice flag for UART receive
        sendByte(configChoice);     //Dynamic RXed character

        break;
      default:
        break;        
    }
}

//*****************************************************************************
//
//!  \brief Returns the number of bytes in the UART buffer
//!
//!  \param  None
//!
//!  \return none
//
//*****************************************************************************
char bytesInUart()
{
    return bytesAvail;    
}

//*****************************************************************************
//
//!  \brief Removes the last byte in the RX buffer if one exists
//!
//!  \param  None
//!
//!  \return none
//
//*****************************************************************************
void removeLastByteinBuf()
{
    if(bytesAvail > 0)
    {
        uartRXBuf[bytesAvail-1] = 0;
        bytesAvail--;
    }
}

//*****************************************************************************
//
//!  \brief Returns a byte in the RX buffer at the specified position
//!
//!  \param  pos is the position
//!
//!  \return the byte at the position, or 0 if the position is invalid
//
//*****************************************************************************
char uartRXByte(char pos)
{
    if(pos <= bytesAvail)
        return uartRXBuf[pos]; 
    else
        return 0;   
}

//*****************************************************************************
//
//!  \brief Resets the UART buffer and pointer
//!
//!  \param  none
//!
//!  \return none
//
//*****************************************************************************
void resetUARTBuffer()
{
    bytesAvail = 0;
    memset(uartRXBuf,0,sizeof(uartRXBuf));    
}

//*****************************************************************************
//
//!  \brief returns a pointer to the RX buffer at the specific position
//!
//!  \param  pos is the location in the RX buffer to which pointer will point
//!
//!  \return a pointer to the RX UART buffer
//
//*****************************************************************************
char *uartRXBytePointer(char pos)
{
    if(pos <= bytesAvail)
        return &uartRXBuf[pos]; 
    else
        return 0;
}
