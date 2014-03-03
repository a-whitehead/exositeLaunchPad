#ifndef __MSPINIT
#define __MSPINIT


#ifdef  __cplusplus
extern "C" {
#endif

extern void msp_init(void);
extern void msp_reinit(void);

/* external peripheral initialization functions */
extern void GPIO_graceInit(void);
extern void BCSplus_graceInit(void);
extern void USCI_A0_graceInit(void);
extern void USCI_B0_graceInit(void);

 extern void System_graceInit(void);
extern void Timer0_A3_graceInit(void);
extern void Timer1_A3_graceInit(void);
extern void Timer2_A3_Init(void);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif

