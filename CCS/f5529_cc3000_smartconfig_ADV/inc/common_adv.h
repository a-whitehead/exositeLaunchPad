/*
 * common_adv.h
 *
 *  Created on: Jan 29, 2014
 *      Author: a0221619
 */

#ifndef COMMON_ADV_H_
#define COMMON_ADV_H_

#include "cc3000_common.h"
#include "socket.h"

// local defines
#define ExositeAppVersion                  "  v0.1  "
#define WRITE_INTERVAL 0
#define EXO_BUFFER_SIZE 200		//reserve 200 bytes for packing all our write data into a buffer
#ifdef __MSP430F5529__
char exo_buffer[EXO_BUFFER_SIZE];
#elif __IAR_SYSTEMS_ICC__
#pragma location = "EXO_DATA"
__no_init char exo_buffer[EXO_BUFFER_SIZE];
#endif

// functions
unsigned char checkWiFiConnected(void);
void show_status(void);
void initNetworkConfig(void);
void initNetworkInitRead(char passPASS[], int flashAddr, int sizePASS);

// externs
extern const char sensorNames[10][11];
extern char *itoa(int n, char *s, int b);
extern void board_init(void);
extern void sendByte(char b);

// global variables
char configChoice;
char serverErrorCode = 0;
int cloud_status = -1;
int configFlag = 0;
int expireCount = 0;														//increments every 500ms
long serverSocket;
static unsigned char obtainIpInfoFlag = FALSE; 								//Flag indicating whether to print CC3000 Connection info
unsigned char * SmartConfigProfilestored = (unsigned char *)0x1848; 		//Flag to indicate Smart Config was performed in the past and CC3000 has a stored profile
unsigned char ConnectUsingSmartConfig = 0;
unsigned char * ptrFtcAtStartup = (unsigned char *)0x1848; 					//Flag to indicate Smart Config needs to be performed
unsigned char dataPacket[] = { '\r', 0xBE, 128, 128, 128, 70, 36, 0xEF }; 	// Definition of data packet to be sent by server
volatile unsigned long ulCC3000Connected;
volatile unsigned long SendmDNSAdvertisment;
volatile char runSmartConfig = 0; 											// brief Flag indicating whether user requested to perform Smart Config
sockaddr serverSocketAddr;
#ifdef EN_COM_CONFIG
	char DevServname[16];
	extern char passSSID[32];
	extern char passPASS[32];
	extern char passSECU[1];
	extern char passMAC[12];
	extern char passDEVNAME[32];
	extern char passMODNAME[32];
	extern int PTRpassSSID;
	extern int PTRpassPASS;
	extern int PTRpassSECU;
	extern int PTRpassMAC;
	extern int PTRpassCIK;
	extern int PTRpassDEVNAME;
	extern int PTRpassMODNAME;
#endif
#ifndef EN_COM_CONFIG
	char DevServname[] = {'C','C','3','0','0','0'};	//Device name - used for Smart config in order to stop the Smart phone configuration process // HARDCODED - Device Name
#endif

#endif /* COMMON_ADV_H_ */
