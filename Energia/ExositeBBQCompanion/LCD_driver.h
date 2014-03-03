/*-
 * Name: LCD_driver.h
 * Introduction: the driver codes for LCD NHD-C0216CZ-NSW-BBW-3V3
 * Created on: Dec 11, 2012
 * Author: a0219294
 */

#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_
#define __MSP430F5529__

#ifdef  __cplusplus
extern "C" {
#endif

extern void LCD_init(void);			//initialize LCD
extern void LCD_clear(void);			//Clean LCD
extern void LCD_display_string(unsigned char L, char *ptr);
extern void LCD_display_number(unsigned char H,unsigned char L,unsigned int num);
extern void LCD_display_temp(unsigned char H,unsigned char L,unsigned int num);
extern void LCD_display_HEX(unsigned char H,unsigned char L,unsigned int num);
extern void LCD_display_char(unsigned char H,unsigned char L,char ch);
extern void LCD_display_time(unsigned char H,unsigned char L,unsigned long seconds);

extern void LCD_writecom(unsigned char c);		//write command
extern void LCD_writedata(unsigned char d);	//write data
extern void LCD_delay_Nms(unsigned char i);	//delay i ms

#ifdef __MSP430G2553__ 
#define LCD_CS_LOW 		P2OUT &= ~BIT5;	//set CS low
#define LCD_CS_HIGH 	P2OUT |= BIT5;	//set CS high
#define LCD_RS_LOW 		P2OUT &= ~BIT4;	//set RS low
#define LCD_RS_HIGH 	P2OUT |= BIT4;	//set RS high
#define LCD_RST_LOW 	P2OUT &= ~BIT3;	//set RST low
#define LCD_RST_HIGH 	P2OUT |= BIT3;	//set RST high
#endif

#ifdef __MSP430F5529__

#define LCD_CS_LOW 		P2OUT &= ~BIT6;	//set CS low
#define LCD_CS_HIGH 	P2OUT |= BIT6;	//set CS high
#define LCD_RS_LOW 		P2OUT &= ~BIT3;	//set RS low
#define LCD_RS_HIGH 	P2OUT |= BIT3;	//set RS high
#define LCD_RST_LOW 	P8OUT &= ~BIT1;	//set RST low
#define LCD_RST_HIGH 	P8OUT |= BIT1;	//set RST high
#endif

// Set CS high to end transaction

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif /* LCD_DRIVER_H_ */
