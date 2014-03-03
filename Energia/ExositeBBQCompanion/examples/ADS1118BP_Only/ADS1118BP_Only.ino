#include <ADS1118.h>
#include <adstemp.h>
#include <board.h>
#include <LCD_driver.h>
#include <mspinit.h>
#include <UART_TxRx.h>

void setup()
{
  Ads.adsinit();
  Ads.adssettime(1,0,10);
  
  Ads.LCDstring(0,"TH:");
  Ads.LCDnumber(0,3,100);
  Ads.LCDstring(1,"Temp:        CH1");
  Ads.LCDchar(1,10,0xDF);
  Ads.LCDchar(1,11,'F');
}

void loop()
{
float value;
int result;
value= Ads.adsReadtemp('f');
result= Ads.adsComp(100,'F');
/*
if(result==1)
{
  Ads.adsBuzz(HIGH);
  Ads.adsDelay(10);
  Ads.adsBuzz(LOW);
}
*/
Ads.adsStarttime();
}
