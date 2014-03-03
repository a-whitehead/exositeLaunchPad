#include <adstemp.h>
#define __MSP430F5529__

//#define HIGH 1
//#define LOW  0

volatile unsigned int  flag;
volatile int total;
volatile int totalExpire;

void AdsClass::adsinit(void)
{
	msp_init();
#ifdef __MSP430G2553__ 
    P1OUT = 0x09;
    P2OUT = 0x3F;
#endif
#ifdef __MSP430F5529__
    //P6OUT |= BIT5;
	//P1OUT |= BIT1;
#endif
	LCD_init() ;
	LCD_clear();
}

void AdsClass::adsreinit(void)
{
	msp_reinit();
}

float AdsClass::adsReadtemp(char mode) // Read temperature from ADS1118, Channel 0 with thermocouple attached
{
    static int local_data, far_data;
	float temp=0;
        float cpy;
        static  int localtemp;
        
        int i=2; // Setting i value to ensure that calibration is also taken into account to calculate the temperature
    
       while(i)
    {

       delay(100);
       localtemp = ADS_Read(1);	//read local temperature data,and start a new convertion for far-end temperature sensor.
        i--;

       delay(100);

    	    far_data = ADS_Read(0);		//read far-end temperature,and start a new convertion for local temperature sensor.
    	    temp = far_data + local_compensation(localtemp); 	// transform the local_data to compensation codes of far-end.
            temp = ADC_code2temp(temp);	// transform the far-end thermocouple codes to temperature.
            cpy=temp;
    	    if(mode== 'C')
            {
				temp = temp;
				cpy=temp;
				LCD_display_temp(1,5, temp);
				cpy=cpy/10;
				return cpy;             
            }
            else
			{ 
				temp = temp * 9 / 5 + 320; 
				cpy=temp;
				LCD_display_temp(1,5, temp);
				cpy=cpy/10;
			return cpy;
			}
      }
    return 0;
}

float AdsClass::adsReadchannel(int channel)
{
	int out = 0;
    int i=1;
	while(i){
		if(channel==2){ // channel two and ground
		  while (!(flag & BIT4));
		  flag&= ~BIT4;
		  out= ADS_Read(2);
		  LCD_display_temp(1,5,out);
		  i-- ;
        }
		if(channel==3){
			while (!(flag & BIT4));
            flag&= ~BIT4;
			out= ADS_Read(3);
			LCD_display_temp(1,5,out);
        }
    }
    return out ;
 }

int AdsClass::adsComp( float value, char mode)
  {
    float temp;
    if(mode=='C')
       temp=adsReadtemp('C');
    else
        temp=adsReadtemp('F');
    if( temp < value)
         return 0;
    else
         return 1;
  }

void AdsClass ::adssettime(int hour, int mins, int secs) // Convert the set time into total number of seconds
{
    total= (hour*3600)+(mins*60)+secs;
}

int AdsClass ::adsStarttime(void)
{
	if(flag & BIT3)
	{
		if (total)
		{
			flag &= ~BIT3;
			// if(total <= 0)	// if current is more than 24 hours
			// {
				// total = 0;
			// }
			// if(total >= 86400)	// if current is more than 24 hours
			// {
				// total = 86400;
			// }			
			LCD_display_time(0,8,total);		// display time on LCD
			return 0;
		}
		return 1;
	}
	return 0;
}


void AdsClass::adsBuzz(int value) // Setting the Buzzer HIGH & LOW
{
#ifdef __MSP430G2553__     
	if(value== HIGH)
         P1OUT &= ~BIT0;
    else
         P1OUT |= BIT0;
#endif
#ifdef __MSP430F5529__		 
	if(value== HIGH)
         P6OUT &= ~BIT5;
    else
         P6OUT |= BIT5;
#endif
}

// Display string on LCD

void AdsClass::LCDstring(unsigned char L, char *ptr)
{
LCD_display_string( L,ptr);
}

// Display number on LCD
void AdsClass::LCDnumber(unsigned char H,unsigned char L,unsigned int num)
{
LCD_display_number(H,L,num);
}

// Display hex on LCD
void AdsClass::LCDhex(unsigned char H, unsigned char L, unsigned num)
{
LCD_display_HEX(H,L,num);
}

void AdsClass::LCDtemp(unsigned char H, unsigned char L, unsigned int temp)
{
LCD_display_temp(H,L,temp);
}

void AdsClass::LCDtime(unsigned char H, unsigned char L,unsigned int seconds )
{
LCD_display_time( H, L, seconds);
}

void AdsClass::LCDchar(unsigned char L,unsigned char H,char ch)
{
LCD_display_char( L,H,ch);
}

void AdsClass::LCDclear(void)
{
LCD_clear( );
}

void AdsClass::adsDelay(int time)
{
enableWatchDog();
delay(time);
disableWatchDog();
}

AdsClass Ads;
