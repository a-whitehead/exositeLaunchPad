#ifndef ADSTEMP_h
#define ADSTEMP_h

#include <msp430.h>
#include <Energia.h>
#include "ADS1118.h"
#include "mspinit.h"
#include"LCD_driver.h"
#include <board.h>
#include <UART_TxRx.h>
#include "Energia.h"
#define __MSP430F5529__

class AdsClass
{
	public :
		void  adsinit(void);
		void  adsreinit(void);
		float adsReadtemp(char);                  
		float adsReadchannel(int);
		void  adssettime(int,int,int);
		void  adsBuzz(int value);
		int   adsStarttime(void);
		void LCDstring(unsigned char L, char *ptr);
		void LCDnumber(unsigned char H,unsigned char L, unsigned int num);
		void LCDhex(unsigned char H, unsigned char L, unsigned int num);
		void LCDtemp(unsigned char H,unsigned char L,unsigned int temp);
		void LCDtime(unsigned char H,unsigned char L,unsigned int seconds);
		void LCDchar(unsigned char L, unsigned char H, char ch);
		void LCDclear(void);
		void adsDelay(int time); 
		int adsComp(float value, char mode);
};

extern AdsClass Ads;

#endif 
