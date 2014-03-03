#include <msp430.h>
#include <common.h>
#include "string.h"
#include "utils.h"
#include "../exosite/exosite.h"

// local
void printNetworkInfo(char passPASS[], int flashAddr, int sizePASS, int fwiMax);
void writeNetworkInfo(int flashAddr, int bytesAvailMax);//(char passPASS[], int flashAddr, int sizePASS, int fwiMax);
void write_Flash();
void read_Flash(char *);
// Flash write
unsigned int fwi;
char configValue;
char * Flash_ptr;                         // Initialize Flash pointer
// Arrays for network information
char passTemp[42];
char passCIK[42];
char passSSID[32];
char passPASS[32];
char passSECU[1];
char passMAC[12];
char passDEVNAME[32];
char passMODNAME[32];
// Pointers to arrays of network information
int PTRpassSSID = 0x1800;
int PTRpassPASS = 0x1820;
int PTRpassSECU = 0x1840;
int PTRpassMAC = 0x1850;
int PTRpassCIK = 0x1880;
int PTRpassDEVNAME = 0x1900;
int PTRpassMODNAME = 0x1920;

//external
extern void sendString(char * msg);
extern void restartMSP430();
extern char configChoice;
extern char uartRXBuf[];
extern char USER_CIK[43];
extern int bytesAvail;
extern int configFlag;
// configFlag to indicate manual network configuration statuses:
 /*
 Bit0 = user has already configured network settings
 Bit1 = UART RX interrupt has received input from COM port
 Bit2 = erase flash memory section for network credentials
 Bit3 = erase flash memory section for CIK
 Bit4 = erase flash memory section for Device/Model Names
 Bit5 = ignore UART RX return character
 Bit6 = expiration timer for dynamic prompt pending
 Bit7 = expiration timer for waiting to connect to access point or smartConfig button push (null by default)
 Bit8 = expiration timer for exosite r/w while connected
 Bit9 = experation timer for Exosite Activate pending
 BitA = ensures COM port prints timestamp only once per Exosite Read/Write
 BitB = ensures SmartConfig button does not get triggered until reaching the appropriate window
 */

#ifdef EN_COM_CONFIG
//*****************************************************************************
//
//!  initNetworkConfig
//!
//!  \param  None
//!
//!  \return None
//!
//!  \brief  Read MAC from Flash due to dynamic network configuration
//
//*****************************************************************************
void initNetworkInitRead(char passPASS[], int flashAddr, int sizePASS)
{
	  bytesAvail = sizePASS;			//12;
	  Flash_ptr = (char *) flashAddr;	//0x1850;
	  read_Flash(passPASS);
}

//*****************************************************************************
//
//!  initNetworkConfig
//!
//!  \param  None
//!
//!  \return None
//!
//!  \brief  Loads program with previously stored or updated credentials
//
//*****************************************************************************
void initNetworkConfig()
{
	// *** START DYNAMIC NETWORK CONFIGURATION ***
	 // Check if user has already configured network settings
	 if (!(configFlag & BIT0)) 		// If so, skip manual network configuration
	 {								// If not, enter manual network configuration
	 sendString("\tStarting Dynamic Network Configuration...\r\n");
	 //Does user have previously stored network configuration information from previous run?
		sendString("\tUser has previously stored network credentials of:\r\n");

		sendString("\t\tCIK:\t\t");
		printNetworkInfo(passCIK, PTRpassCIK, sizeof(passCIK), 40);
		sendString(passCIK);
		passCIK[40] = '\r';
		passCIK[41] = '\n';
		memcpy((char*)USER_CIK, passCIK, 43);

		sendString("\r\n\t\tDEVICE NAME:\t");
	  	printNetworkInfo(passDEVNAME, PTRpassDEVNAME, sizeof(passDEVNAME), 32);
		sendString(passDEVNAME);

		sendString("\r\n\t\tMODEL NAME:\t");
		printNetworkInfo(passMODNAME, PTRpassMODNAME, sizeof(passMODNAME), 32);
		sendString(passMODNAME);

		sendString("\r\n\t\tSSID:\t\t");
		printNetworkInfo(passSSID, PTRpassSSID, sizeof(passSSID), 32);
		sendString(passSSID);

		sendString("\r\n\t\tPASSPHRASE:\t");
		printNetworkInfo(passPASS, PTRpassPASS, sizeof(passPASS), 32);
		if (fwi>0)
		{
			memset(passTemp,'*',fwi);
		}
		sendString(passTemp);

		sendString("\r\n\t\tSECURITY:\t");
		printNetworkInfo(passSECU, PTRpassSECU, sizeof(passSECU), 8);
		if (passSECU[0] == '0')
		{
			sendString("UNSEC");
		}
		else if (passSECU[0] == '1')
		{
			sendString("WEP");
		}
		else if (passSECU[0] == '2')
		{
			sendString("WPA");
		}
		else if (passSECU[0] == '3')
		{
			sendString("WPA2");
		}
		else;

		sendString("\r\n\t\tMAC ADDRESS:\t");
		printNetworkInfo(passMAC, PTRpassMAC, sizeof(passMAC), 16);
		sendString(passMAC);

	sendString("\r\n\tUpdate Settings? (y/n)\r\n\t");
	while((configChoice!='n') && (configChoice!='y'))
	{
		while (!(configFlag & BIT1));
		configFlag &= ~BIT1;
		configFlag &= ~BIT6;
			if (configChoice=='y')
			{
				configChoice='.';
				sendString("\r\n\tUpdate Exosite CIK? (y/n)\r\n\t");
				while((configChoice!='n') && (configChoice!='y'))
					{
					while (!(configFlag & BIT1));
					configFlag &= ~BIT1;
						if (configChoice=='y')
						{
							configFlag |= BIT3; //Erase flash memory section for CIK
							sendString("\r\n\tEnter CIK:\r\n\t");
							writeNetworkInfo(PTRpassCIK, 40);
							break;
						}
						else if(configChoice=='n')
						{
							break;
						}
						else
						{
							sendString("\b");
						}
					}
				configChoice='.';

				sendString("\r\n\tUpdate Device and Model Names? (y/n)\r\n\t");
				while((configChoice!='n') && (configChoice!='y'))
					{
					while (!(configFlag & BIT1));
					configFlag &= ~BIT1;
						if (configChoice=='y')
						{
							configFlag |= BIT4; //Erase flash memory section for Dev/Model Names
							sendString("\r\n\tEnter Device Name:\r\n\t");
							writeNetworkInfo(PTRpassDEVNAME, 16);
							sendString("\r\n\tEnter Model Name:\r\n\t");
							writeNetworkInfo(PTRpassMODNAME, 32);
							break;
						}
						else if(configChoice=='n')
						{
							break;
						}
						else
						{
							sendString("\b");
						}

					}
				configChoice='.';

				sendString("\r\n\tUpdate Network Credentials (SSID, PASSPHRASE, SECURITY)? (y/n)\r\n\t");
				while((configChoice!='n') && (configChoice!='y'))
				{
					while (!(configFlag & BIT1));
					configFlag &= ~BIT1;
						if (configChoice=='y')
						{
							configFlag |= BIT2; //Erase flash memory section for network credentials
							sendString("\r\n\tEnter SSID:\r\n\t");
							writeNetworkInfo(PTRpassSSID, 32);
							sendString("\r\n\tEnter PASSPHRASE:\r\n\t");
							writeNetworkInfo(PTRpassPASS, 32);

							sendString("\r\n\tEnter SECURITY: (0=WLAN_SEC_UNSEC, 1=WLAN_SEC_WEP, 2=WLAN_SEC_WPA, 3=WLAN_SEC_WPA2)\r\n\t");
							bytesAvail=0;
							while((((configChoice!='0') && (configChoice!='1') && (configChoice!='2') && (configChoice!='3'))) && !(bytesAvail>=1))
							{
								bytesAvail=0;
								while (!(configFlag & BIT1));
								configFlag &= ~BIT1;
								memset(uartRXBuf, 0, 50);
								uartRXBuf[0] = configChoice;
									if((configChoice=='0') || (configChoice=='1') || (configChoice=='2') || (configChoice=='3'))
									{
										Flash_ptr = (char *) PTRpassSECU;
										_disable_interrupt();
										write_Flash();
										_enable_interrupt();
										//writeNetworkInfo(PTRpassSECU); //FACTORY ONLY
									}
									else
									{
										sendString("\b");
									}
							}
							break;
						}
						else if(configChoice=='n')
						{
							break;
						}
						else
						{
							sendString("\b \b");
						}
				}
				configChoice='.';

				sendString("\r\n\tUpdate MAC Address? (y/n)\r\n\t");
				configChoice='.';
				while((configChoice!='n') && (configChoice!='y'))
				{
					while (!(configFlag & BIT1));
					configFlag &= ~BIT1;
					memset(uartRXBuf, 0, 50);
					Flash_ptr = (char *) 0x1850;
						if (configChoice=='y')
						{
							sendString("\r\n\tEnter MAC ADDRESS:\r\n\t");
							writeNetworkInfo(PTRpassMAC, 12);
							break;
						}
						else if(configChoice=='n')
						{
							//Rewrite previous MAC address
							for(fwi = 0; fwi < sizeof(passMAC); fwi++)
							{
								uartRXBuf[fwi] = passMAC[fwi];
							}
							bytesAvail = 12;
							_disable_interrupt();
							write_Flash();
							_enable_interrupt();
							break;
						}
						else
						{
							sendString("\b");
						}
				}
				sendString("\r\n\tCredentials have been updated.\r\n");
				configChoice='y';
				busyWait(100);
				restartMSP430();
			}
			else if(configChoice=='n')
			{
				sendString("\r\n\tProgram will use previously stored credentials.\r\n");
				break;
			}
			else
			{
				sendString("\b \b");
			}
	 }
	configFlag |= BIT0; // Skip network configuration on next while loop iteration
	// *** END DYNAMIC NETWORK CONFIGURATION ***
	 }
}

//------------------------------------------------------------------------------
// Print previously stored network configuration info
//------------------------------------------------------------------------------
void printNetworkInfo(char passPASS[], int flashAddr, int sizePASS, int fwiMax)
{
	memset(passPASS, 0, sizePASS);
	Flash_ptr = (char *) flashAddr;
	for (fwi = 0; fwi < fwiMax; fwi++)
	{
		configValue = *Flash_ptr;
		if(configValue =='ÿ') //equivalent to HEX=0xFF, DEC=13)
		{
			break;
		}
		passPASS[fwi] = configValue;
		*Flash_ptr++;
	}
}

//------------------------------------------------------------------------------
// Write network configuration info
//------------------------------------------------------------------------------
void writeNetworkInfo(int flashAddr, int bytesAvailMax)
{
	memset(uartRXBuf, 0, 50);
	memset(passTemp, 0, 42);
	bytesAvail=0;
	while (((!(configFlag & BIT1)) || (configChoice!=13)) && !(bytesAvail>=bytesAvailMax));
	configFlag &= ~BIT1;
	memcpy(passTemp,uartRXBuf,bytesAvail);
	Flash_ptr = (char *) flashAddr;
	_disable_interrupt();
	write_Flash();
	_enable_interrupt();
	memset(uartRXBuf, 0, 50);
	memset(passTemp, 0, sizeof(passTemp));
}

//------------------------------------------------------------------------------
// Input = value, holds value to write to Seg //Ref p.359
//------------------------------------------------------------------------------
void write_Flash()
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit

  //Erase network credentials
  if(configFlag & BIT2)
  {
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT2;
  }
  //Erase CIK
  if(configFlag & BIT3)
  {
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT3;
  }
  //Erase Device and Model Names
  if(configFlag & BIT4)
  {
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT4;
  }

  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

  for (fwi = 0; fwi < bytesAvail; fwi++)
  {
	  configValue = uartRXBuf[fwi];
	  *Flash_ptr++ = configValue;                   // Write value to flash
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}

//------------------------------------------------------------------------------
// Input = value, holds value to write to Seg //Ref p.359
//------------------------------------------------------------------------------
void read_Flash(char * flashBuf)
{
  FCTL3 = FWKEY;                            // Clear Lock bit

  //Erase network credentials
  if(configFlag & BIT2)
  {
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT2;
  }
  //Erase CIK
  if(configFlag & BIT3)
  {
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT3;
  }
  //Erase Device and Model Names
  if(configFlag & BIT4)
  {
	  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
	  configFlag &= ~BIT4;
  }

  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

  for (fwi = 0; fwi < bytesAvail; fwi++)
  {
	  *flashBuf++ = *Flash_ptr++;               // Write value to flash
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}

#endif
