/******************************************************************************
*	Biblioteca: LaunchPad
*	Autor: Leandro Poloni Dantas
*	Data: 17/03/18
*	Sobre: Est� biblioteca foi criada com o objetivo de colecionar fun��es
*		   criadas para facilitar a utiliza��o da LaunchPad TM4C123 da Texas.
*	Vers�o: 1.0
*	Revis�es:
*
******************************************************************************/

//*****************************************************************************
//	Defini��es e Apelidos
//*****************************************************************************
#ifndef LaunchPad_H
#define LaunchPad_H
#endif

//GPIOF	4		3			2		 1	 0
//			SW1	Green	Blue Red SW2

//M�scara de AND para testar um bit
//Exemplo bit 4 (GPIOF SW1)
//Valor     xxxx xxxx
//M�scara   0001 0000
//          --------- AND
//Resultado 000x 0000
//Se Resultado > 0 o bit est� setado
//Se Resultado = 0 o bit est� resetado
//M�scara de OR/AND para setar/resetar bit
//Exemplo bit 2 (GPIOF Red)
//Liga: 	 GPIOF->DATA = GPIOF->DATA | Red;
//Desliga: GPIOF->DATA = GPIOF->DATA & ~Red;

#define SW1 		0x10
#define SW2 		0x01
#define Red 		0x02
#define Green 	0x08
#define Blue 		0x04


//*****************************************************************************
//	Prot�tipos de Fun��es
//*****************************************************************************
void GPIOF_Inicializa(void);
void LedRed_Inicializa(void);
void UART0_Inicializa_115200bps_80MHz(void);
void UART0_Inicializa_115200bps_16MHz(void);
void PLL_Inicializa(void);
void SysTick_Inicializa(void);
void SysTick_Inicializa_Int(unsigned long period);
void SysTick_Wait(unsigned long delay);
void SysTick_Wait10ms(unsigned long delay);
unsigned char UART0_RxChar(void);
void UART0_TxChar(unsigned char data);
void UART0_TxString(unsigned char* data);
void ADC0_InitSWTriggerSeq3_Ch1(void);
unsigned long ADC0_InSeq3(void);
void EdgeCounter_Init(void);
void Servo_Init(void);
void Servo_Set_Angle(int angulo);
void WDT_Init(void);
void IntToString(unsigned int n, char* s);
void IntToStringHexa(unsigned int n, char* s);
unsigned int StringToInt(unsigned char* s);
void Timer0_Init(void);
