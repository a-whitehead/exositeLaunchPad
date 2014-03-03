#include<msp430.h>
#include "mspinit.h"
#define __MSP430F5529__

void msp_init(void)
{
    /* initialize Config for the MSP430 GPIO */
    GPIO_graceInit();

    /* initialize Config for the MSP430 USCI_B0 */
    USCI_B0_graceInit();

    /* initialize Config for the MSP430 A3 Timer0 */
    Timer0_A3_graceInit();
}

void msp_reinit(void)
{
	USCI_B0_graceInit(); //needed to loop ADS1118 measurements w/ CC3000
}


//************************ Initialize all Gpio's******************************************

void GPIO_graceInit(void)
{
    #ifdef __MSP430G2553__ 
	/* USER CODE START (section: GPIO_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: GPIO_graceInit_prologue) */

    /* Port 1 Port Select 2 Register */
    P1SEL2 = BIT1 | BIT2 | BIT5 | BIT6 | BIT7;

    /* Port 1 Output Register */
    P1OUT = BIT0 | BIT3;

    /* Port 1 Port Select Register */
    P1SEL = BIT1 | BIT2 | BIT5 | BIT6 | BIT7;

    /* Port 1 Direction Register */
    P1DIR = BIT0 | BIT4;

    /* Port 1 Resistor Enable Register */
    P1REN = BIT3;

    /* Port 1 Interrupt Edge Select Register */
    P1IES = BIT3;

    /* Port 1 Interrupt Flag Register */
    P1IFG = 0;

    /* Port 1 Interrupt Enable Register */
    P1IE = BIT3;

    /* Port 2 Output Register */
    P2OUT = BIT1 | BIT2;

    /* Port 2 Port Select Register */
    P2SEL &= ~(BIT6 | BIT7);

    /* Port 2 Direction Register */
    P2DIR = BIT0 | BIT3 | BIT4 | BIT5;

    /* Port 2 Resistor Enable Register */
    P2REN = BIT1 | BIT2;

    /* Port 2 Interrupt Edge Select Register */
    P2IES = BIT1 | BIT2;

    /* Port 2 Interrupt Flag Register */
    P2IFG = 0;

    /* Port 2 Interrupt Enable Register */
    P2IE = BIT1 | BIT2;

    /* Port 3 Output Register */
    P3OUT = 0;

    /* Port 3 Port Select Register */
    P3SEL = BIT1;

    /* Port 3 Direction Register */
    P3DIR = BIT1;

    /* USER CODE START (section: GPIO_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: GPIO_graceInit_epilogue) */
	
	#endif
	#ifdef __MSP430F5529__
	/* USER CODE START (section: GPIO_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: GPIO_graceInit_prologue) */

    /* Port 1 Port Select 2 Register */
    //P1SEL2 = BIT1 | BIT2 | BIT5 | BIT6 | BIT7;


    /* Port 1 Output Register */
    P6OUT = BIT5; P1OUT = BIT6; P1OUT = BIT1;

    /* Port 1 Port Select Register */
    P3SEL = BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

    /* Port 1 Direction Register */
    P6DIR = BIT5 | BIT6;

    /* Port 1 Resistor Enable Register */
    P1REN |= BIT1;

    /* Port 1 Interrupt Edge Select Register */
    P1IES |= BIT1;

    /* Port 1 Interrupt Flag Register */
    P1IFG |= 0;

    /* Port 1 Interrupt Enable Register */
    P1IE |= BIT1;

    /* Port 2 Port Select Register */
    P2SEL &= ~(BIT0 | BIT2);

    /* Port 2 Direction Register */
    P2DIR = BIT7 | BIT3 | BIT6; P8DIR = BIT1;

    /* Port 2 Resistor Enable Register */
    P4REN = BIT2 | BIT1;

    /* Port 2 Interrupt Flag Register */
    UCB1IFG = 0;

    /* Port 2 Interrupt Enable Register */
    UCB1IE |= UCTXIE + UCRXIE;

    /* USER CODE START (section: GPIO_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: GPIO_graceInit_epilogue) */
	#endif
}

/*
 *  ======== BCSplus_graceInit ========
 *  Initialize MSP430 Basic Clock System
 */
void BCSplus_graceInit(void)
{
    #ifdef __MSP430G2553__ 
	/* USER CODE START (section: BCSplus_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: BCSplus_graceInit_prologue) */

    /*
     * Basic Clock System Control 2
     *
     * SELM_0 -- DCOCLK
     * DIVM_0 -- Divide by 1
     * ~SELS -- DCOCLK
     * DIVS_1 -- Divide by 2
     * ~DCOR -- DCO uses internal resistor
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_1;

    if (CALBC1_1MHZ != 0xFF) {
        /* Follow recommended flow. First, clear all DCOx and MODx bits. Then
         * apply new RSELx values. Finally, apply new DCOx and MODx bit values.
         */
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
        DCOCTL = CALDCO_1MHZ;
    }

    /*
     * Basic Clock System Control 1
     *
     * XT2OFF -- Disable XT2CLK
     * ~XTS -- Low Frequency
     * DIVA_3 -- Divide by 8
     *
     * Note: ~XTS indicates that XTS has value zero
     */
    BCSCTL1 |= XT2OFF | DIVA_3;

    /*
     * Basic Clock System Control 3
     *
     * XT2S_0 -- 0.4 - 1 MHz
     * LFXT1S_0 -- If XTS = 0, XT1 = 32768kHz Crystal ; If XTS = 1, XT1 = 0.4 - 1-MHz crystal or resonator
     * XCAP_3 -- ~12.5 pF
     */
    BCSCTL3 = XT2S_0 | LFXT1S_0 | XCAP_3;

    /* USER CODE START (section: BCSplus_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: BCSplus_graceInit_epilogue) */
	
	#endif
	#ifdef __MSP430F5529__
    UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;                        // Set ACLK = REFO
	UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx

	// Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
	do
	{
	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
											// Clear XT2,XT1,DCO fault flags
	SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	__bis_SR_register(SCG0);                  // Disable the FLL control loop
	UCSCTL1 = DCORSEL_0;                      // Select DCO range 16MHz operation
	UCSCTL2 |= 29;                           // Set DCO Multiplier for 8MHz
											// (N + 1) * FLLRef = Fdco
											// (249 + 1) * 32768 = 8MHz
	__bic_SR_register(SCG0);                  // Enable the FLL control loop

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
	__delay_cycles(250000);
	
	#endif
}


/*
 *  ======== USCI_A0_graceInit ========
 *  Initialize Universal Serial Communication Interface A0 UART 2xx
 */
void USCI_A0_graceInit(void)
{
    /* USER CODE START (section: USCI_A0_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: USCI_A0_graceInit_prologue) */

    /* Disable USCI */
    UCA0CTL1 |= UCSWRST;

    /*
     * Control Register 1
     *
     * UCSSEL_2 -- SMCLK
     * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
     * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
     * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
     * ~UCTXADDR -- Next frame transmitted is data
     * ~UCTXBRK -- Next frame transmitted is not a break
     * UCSWRST -- Enabled. USCI logic held in reset state
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCA0CTL1 = UCSSEL_2 | UCSWRST;

    /* Baud rate control register 0 */
    UCA0BR0 = 52;

    /* Enable USCI */
    UCA0CTL1 &= ~UCSWRST;

    /* USER CODE START (section: USCI_A0_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: USCI_A0_graceInit_epilogue) */
}




/*
 *  ======== USCI_B0_graceInit ========
 *  Initialize Universal Serial Communication Interface B0 SPI 2xx
 */
void USCI_B0_graceInit(void)
{
    /* USER CODE START (section: USCI_B0_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: USCI_B0_graceInit_prologue) */

    /* Disable USCI */
    UCB0CTL1 |= UCSWRST;

    /*
     * Control Register 0
     *
     * ~UCCKPH -- Data is changed on the first UCLK edge and captured on the following edge
     * UCCKPL -- Inactive state is high
     * UCMSB -- MSB first
     * ~UC7BIT -- 8-bit
     * UCMST -- Master mode
     * UCMODE_0 -- 3-Pin SPI
     * UCSYNC -- Synchronous Mode
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCB0CTL0 = UCCKPL | UCMSB | UCMST | UCMODE_0 | UCSYNC;

    /*
     * Control Register 1
     *
     * UCSSEL_2 -- SMCLK
     * UCSWRST -- Enabled. USCI logic held in reset state
     */
    UCB0CTL1 = UCSSEL_2 | UCSWRST;

    /* Bit Rate Control Register 0 */
    UCB0BR0 = 100;

    /* Enable USCI */
    UCB0CTL1 &= ~UCSWRST;

    /* USER CODE START (section: USCI_B0_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: USCI_B0_graceInit_epilogue) */
}

/*
 *  ======== System_graceInit ========
 *  Initialize MSP430 Status Register
 */
void System_graceInit(void)
{
    /* USER CODE START (section: System_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: System_graceInit_prologue) */

    /*
     * SR, Status Register
     *
     * ~SCG1 -- Disable System clock generator 1
     * ~SCG0 -- Disable System clock generator 0
     * ~OSCOFF -- Oscillator On
     * ~CPUOFF -- CPU On
     * GIE -- General interrupt enable
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    __bis_SR_register(GIE);

    /* USER CODE START (section: System_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: System_graceInit_epilogue) */
}



/*
 *  ======== Timer0_A3_graceInit ========
 *  Initialize MSP430 Timer0_A3 timer
 */
void Timer0_A3_graceInit(void)
{
    /* USER CODE START (section: Timer0_A3_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: Timer0_A3_graceInit_prologue) */

    /*
     * TA0CCTL0, Capture/Compare Control Register 0
     *
     * CM_0 -- No Capture
     * CCIS_0 -- CCIxA
     * ~SCS -- Asynchronous Capture
     * ~SCCI -- Latched capture signal (read)
     * ~CAP -- Compare mode
     * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    TA0CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE;

    /* TA0CCR0, Timer_A Capture/Compare Register 0 */
    TA0CCR0 = 31249;

    /*
     * TA0CTL, Timer_A3 Control Register
     *
     * TASSEL_2 -- SMCLK
     * ID_3 -- Divider - /8
     * MC_1 -- Up Mode
     */
    TA0CTL = TASSEL_2 | ID_3 | MC_1; //TA0CTL = TASSEL_2 | ID_3 | MC_1;

    /* USER CODE START (section: Timer0_A3_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: Timer0_A3_graceInit_epilogue) */
}

/*
 *  ======== Timer1_A3_graceInit ========
 *  Initialize MSP430 Timer1_A3 timer
 */
void Timer1_A3_graceInit(void)
{
    /* USER CODE START (section: Timer1_A3_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: Timer1_A3_graceInit_prologue) */

    /*
     * TA1CCTL0, Capture/Compare Control Register 0
     *
     * CM_0 -- No Capture
     * CCIS_0 -- CCIxA
     * ~SCS -- Asynchronous Capture
     * ~SCCI -- Latched capture signal (read)
     * ~CAP -- Compare mode
     * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    TA1CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE;

    /* TA1CCR0, Timer_A Capture/Compare Register 0 */
    //TA1CCR0 = 6249;
    TA1CCR0 = 12489;

    /*
     * TA1CTL, Timer_A3 Control Register
     *
     * TASSEL_2 -- SMCLK
     * ID_3 -- Divider - /8
     * MC_1 -- Up Mode
     */
    TA1CTL = TASSEL_2 | ID_3 | MC_1;

    /* USER CODE START (section: Timer1_A3_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: Timer1_A3_graceInit_epilogue) */
}

/*
 *  ======== Timer0_A2 ========
 *  Initialize MSP430 Timer0_A2 timer
 */
void Timer2_A3_Init(void)
{
	//TA2CCTL1 &= ~CCIE;
	//TA2CTL |= MC_0;
	// Configure the timer for each 500 milli to handle un-solicited events
	TA2CCR0 = 0x4000;
	// run the timer from ACLCK, and enable interrupt of Timer A
	TA2CTL = TASSEL_1 + MC_1 + TACLR + TAIE + ID_1;  // SMCLK, contmode, clear TAR//TA2CTL |= (TASSEL_1 + MC_1 + TACLR);
	//TA2CCTL1 |= CCIE;
}