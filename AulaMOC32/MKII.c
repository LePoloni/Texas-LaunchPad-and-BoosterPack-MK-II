/******************************************************************************
*	Biblioteca: MK_II
*	Autor: Leandro Poloni Dantas
*	Data: 17/03/19
*	Sobre: Está biblioteca foi criada com o objetivo de colecionar funções
*		     criadas para facilitar a utilização da BoosterPack MK II juntamente
*				 com a LaunchPad TM4C123 da Texas.
*	Versão: 1.0
*	Revisões:
*				26/03/19 - Incluidas as funções para controle do Servo Motor pino PB2
*										(J2.19)
*
******************************************************************************/

//*****************************************************************************
//	Pinagem
//*****************************************************************************
/*
//  J1   J3               J4   J2
// [ 1] [21]             [40] [20]
// [ 2] [22]             [39] [19]
// [ 3] [23]             [38] [18]
// [ 4] [24]             [37] [17]
// [ 5] [25]             [36] [16]
// [ 6] [26]             [35] [15]
// [ 7] [27]             [34] [14]
// [ 8] [28]             [33] [13]
// [ 9] [29]             [32] [12]
// [10] [30]             [31] [11]

// Connected pins in physical order
// J1.1 +3.3V (power)
// J1.2 joystick horizontal (X) (analog) {TM4C123 PB5/AIN11, MSP432 P6.0}
// J1.3 UART from Bluetooth to LaunchPad (UART) {TM4C123 PB0, MSP432 P3.2} 
// J1.4 UART from LaunchPad to Bluetooth (UART) {TM4C123 PB1, MSP432 P3.3} 
// J1.5 joystick Select button (digital) {TM4C123 PE4, MSP432 P4.1}
// J1.6 microphone (analog)              {TM4C123 PE5/AIN8, MSP432 P4.3}
// J1.7 LCD SPI clock (SPI)              {TM4C123 PB4, MSP432 P1.5}
// J1.8 ambient light (OPT3001) interrupt (digital) {TM4C123 PA5, MSP432 P4.6}
// J1.9 ambient light (OPT3001) and temperature sensor (TMP006) I2C SCL (I2C)  {TM4C123 PA6, MSP432 P6.5}
// J1.10 ambient light (OPT3001) and temperature sensor (TMP006) I2C SDA (I2C) {TM4C123 PA7, MSP432 P6.4}
//--------------------------------------------------
// J2.11 temperature sensor (TMP006) interrupt (digital) {TM4C123 PA2, MSP432 P3.6}
// J2.12 nothing                         {TM4C123 PA3, MSP432 P5.2}
// J2.13 LCD SPI CS (SPI)                {TM4C123 PA4, MSP432 P5.0} 
// J2.14 nothing                         {TM4C123 PB6, MSP432 P1.7}
// J2.15 LCD SPI data (SPI)              {TM4C123 PB7, MSP432 P1.6}
// J2.16 nothing (reset)  
// J2.17 LCD !RST (digital)              {TM4C123 PF0, MSP432 P5.7}
// J2.18 Profile 4                       {TM4C123 PE0, MSP432 P3.0}
// J2.19 servo PWM                       {TM4C123 PB2, MSP432 P2.5}
// J2.20 GND (ground)
//--------------------------------------------------
// J3.21 +5V (power)
// J3.22 GND (ground)
// J3.23 accelerometer X (analog)        {TM4C123 PD0/AIN7, MSP432 P6.1}
// J3.24 accelerometer Y (analog)        {TM4C123 PD1/AIN6, MSP432 P4.0}
// J3.25 accelerometer Z (analog)        {TM4C123 PD2/AIN5, MSP432 P4.2}
// J3.26 joystick vertical (Y) (analog)  {TM4C123 PD3/AIN4, MSP432 P4.4}
// J3.27 Profile 0                       {TM4C123 PE1, MSP432 P4.5}
// J3.28 Profile 1                       {TM4C123 PE2, MSP432 P4.7}
// J3.29 Profile 2                       {TM4C123 PE3, MSP432 P5.4}
// J3.30 Profile 3                       {TM4C123 PF1, MSP432 P5.5}
//--------------------------------------------------
// J4.31 LCD RS (digital)                {TM4C123 PF4, MSP432 P3.7}
// J4.32 user Button2 (bottom) (digital) {TM4C123 PD7, MSP432 P3.5}
// J4.33 user Button1 (top) (digital)    {TM4C123 PD6, MSP432 P5,1}
// J4.34 Profile 6/gator hole switch     {TM4C123 PC7, MSP432 P2.3}
// J4.35 nothing                         {TM4C123 PC6, MSP432 P6.7}
// J4.36 Profile 5                       {TM4C123 PC5, MSP432 P6.6}
// J4.37 RGB LED blue (PWM)              {TM4C123 PC4, MSP432 P5.6}
// J4.38 RGB LED green (PWM)             {TM4C123 PB3, MSP432 P2.4}
// J4.39 RGB LED red (jumper up) or LCD backlight (jumper down) (PWM) {TM4C123 PF3, MSP432 P2.6}
// J4.40 buzzer (PWM)                    {TM4C123 PF2, MSP432 P2.7}
//--------------------------------------------------
// Connected pins in logic order
// power and reset
// J1.1 +3.3V (power)
// J3.21 +5V (power)
// J3.22 GND (ground)
// J2.20 GND (ground)
// J2.16 nothing (reset)  
//--------------------------------------------------
// LCD graphics
// J1.7 LCD SPI clock (SPI)              {TM4C123 PB4, MSP432 P1.5}
// J2.13 LCD SPI CS (SPI)                {TM4C123 PA4, MSP432 P5.0} 
// J2.15 LCD SPI data (SPI)              {TM4C123 PB7, MSP432 P1.6}
// J2.17 LCD !RST (digital)              {TM4C123 PF0, MSP432 P5.7}
// J4.31 LCD RS (digital)                {TM4C123 PF4, MSP432 P3.7}
//--------------------------------------------------
// 3-color LED
// J4.37 RGB LED blue (PWM)              {TM4C123 PC4, MSP432 P5.6}
// J4.38 RGB LED green (PWM)             {TM4C123 PB3, MSP432 P2.4}
// J4.39 RGB LED red (jumper up) or LCD backlight (jumper down) (PWM) {TM4C123 PF3, MSP432 P2.6}
//--------------------------------------------------
// user buttons
// J4.32 user Button2 (bottom) (digital) {TM4C123 PD7, MSP432 P3.5}
// J4.33 user Button1 (top) (digital)    {TM4C123 PD6, MSP432 P5.1}
//--------------------------------------------------
// buzzer output
// J4.40 buzzer (PWM)                    {TM4C123 PF2, MSP432 P2.7}
//--------------------------------------------------
// Joystick
// J1.5 joystick Select button (digital) {TM4C123 PE4, MSP432 P4.1}
// J1.2 joystick horizontal (X) (analog) {TM4C123 PB5/AIN11, MSP432 P6.0}
// J3.26 joystick vertical (Y) (analog)  {TM4C123 PD3/AIN4, MSP432 P4.4}
//--------------------------------------------------
// accelerometer
// J3.23 accelerometer X (analog)        {TM4C123 PD0/AIN7, MSP432 P6.1}
// J3.24 accelerometer Y (analog)        {TM4C123 PD1/AIN6, MSP432 P4.0}
// J3.25 accelerometer Z (analog)        {TM4C123 PD2/AIN5, MSP432 P4.2}
//--------------------------------------------------
// microphone
// J1.6 microphone (analog)              {TM4C123 PE5/AIN8, MSP432 P4.3}
//--------------------------------------------------
// light and temperature sensors (I2C)
// J1.8 ambient light (OPT3001) interrupt (digital) {TM4C123 PA5, MSP432 P4.6}
// J1.9 ambient light (OPT3001) and temperature sensor (TMP006) I2C SCL (I2C)  {TM4C123 PA6, MSP432 P6.5}
// J1.10 ambient light (OPT3001) and temperature sensor (TMP006) I2C SDA (I2C) {TM4C123 PA7, MSP432 P6.4}
// J2.11 temperature sensor (TMP006) interrupt (digital) {TM4C123 PA2, MSP432 P3.6}
//--------------------------------------------------
// Bluetooth booster
// J1.3 UART from Bluetooth to LaunchPad (UART) {TM4C123 PB0, MSP432 P3.2} 
// J1.4 UART from LaunchPad to Bluetooth (UART) {TM4C123 PB1, MSP432 P3.3} 
//--------------------------------------------------
// profile pins
// J3.27 Profile 0                       {TM4C123 PE1, MSP432 P4.5}
// J3.28 Profile 1                       {TM4C123 PE2, MSP432 P4.7}
// J3.29 Profile 2                       {TM4C123 PE3, MSP432 P5.4}
// J3.30 Profile 3                       {TM4C123 PF1, MSP432 P5.5}
// J2.18 Profile 4                       {TM4C123 PE0, MSP432 P3.0}
// J4.36 Profile 5                       {TM4C123 PC5, MSP432 P6.6}
// J4.34 Profile 6                       {TM4C123 PC7, MSP432 P2.3}
//--------------------------------------------------
// unconnected pins
// J2.12 nothing                         {TM4C123 PA3, MSP432 P5.2}
// J2.14 nothing                         {TM4C123 PB6, MSP432 P1.7}
// J2.19 servo PWM                       {TM4C123 PB2, MSP432 P2.5}
// J4.35 nothing                         {TM4C123 PC6, MSP432 P6.7}
//*/

//*****************************************************************************
//	Includes
//*****************************************************************************
//CMSIS
#include "TM4C123GH6PM.h"						//CMSIS Cortex-M4 Peripheral Access Layer
//ARMCC (ARM C Compiler)
#include <stdint.h>									//Definição de tipos inteiros, máximos e mínimos
#include <stdbool.h>								//Definição de tipo booleano
//TIVAWARE
//#include "inc/hw_memmap.h"					//Macros defining the memory map of the device (mesmas definições de TM4C123GH6PM.h)
#include "driverlib/fpu.h"					//Prototypes for the floatint point manipulation routines
#include "driverlib/sysctl.h"				//Prototypes for the system control driver
#include "driverlib/rom.h"					//Macros to facilitate calling functions in the ROM
#include "driverlib/pin_map.h"			//Mapping of peripherals to pins for all parts
#include "driverlib/uart.h"					//Defines and Macros for the UART
#include "grlib/grlib.h"						//Prototypes for the low level primitives provided by the graphics library
#include "utils/uartstdio.h"				//Prototypes for the UART console functions
#include "driverlib/gpio.h"					//Defines and Macros for GPIO API
#include "driverlib/timer.h"				//Prototypes for the timer
//Header da biblioteca
#include "MKII.h"
//*****************************************************************************
//	Variáveis Globais
//*****************************************************************************


//*****************************************************************************
//	Funções
//*****************************************************************************

//*****************************************************************************
//Inicializa GPTM (Timer 1 modo PWM - buzzer do BoosterPack MK II)
//Supondo SystemCoreClock = 16 MHz
//freq mínima = 16.000.000 / 65536 = 244 Hz
//freq máxima = 10 kHz (datasheet buzzer CEM-1203)
//duty em % de 1% em 1% 
void MKII_Timer1_PWM_Init(uint16_t freq, uint16_t duty)
{
	if((duty > 100) || (freq > 10000))
		return;                        		//Entrada inválida
  SYSCTL->RCGCTIMER |= 0x02;					//1-Habilita o clock do timer
	
	//Configuração do Port F
	SYSCTL->RCGCGPIO |= 0x0020;     		//1-Ativa o clock do Port F
  while((SYSCTL->PRGPIO&0x20) == 0);	//2-Aguarda o clock estabilizar
  GPIOF->AFSEL |= 0x04;				      	//3-Habilita função alternativa no PF2
  GPIOF->DEN |= 0x04;		        			//4-Habilita I/O difgital no PF2
  GPIOF->PCTL = (GPIOF->PCTL & 0xFFFFF0FF) + 0x00000700;	//5-Seleciona a função T1CCP0 no PF2
  GPIOF->AMSEL &= ~0x04;			 	    	//5-Desabilita função analógica no PF2
	
	//Configuração do Timer 1 modo PWM
	SYSCTL->RCGCTIMER |= 0x02;					//1-Habilita o clock do timer
	while((SYSCTL->PRTIMER&0x02) == 0);	//2-Aguarda o clock estabilizar
	TIMER1->CTL &= ~(1<<0); 						//3-Desabilita Timer1A durante o setup
	TIMER1->CFG = 0x04;									//4-Configura o timer para modo 16 bits
	TIMER1->TAMR |= 1<<3;								//5-Seleciona o modo alternativo PWM (bit 3 - AMS - GPTM Timer A Alternate Mode Select)
	TIMER1->TAMR &= ~(1<<2);						//6-Seleciona o modo contagem de bordas (bit 2 - CMR - GPTM Timer A Capture Mode)
	TIMER1->TAMR |= 0x02;								//7-Configura o Timer0A para modo periódico (bits 1-0 - MR - GPTM Timer A Mode)
	TIMER1->CTL |= 1<<6;								//8-Configura o estado do sinal de PWM (invertido) (bit 6 - TAPWML - GPTM Timer A PWM Output Level)  
	TIMER1->TAPR = 0x00;								//9-Configura o valor do prescaler
	TIMER1->CTL &= ~(3<<2);							//10-Configura o evento de interrupção de PWM (borda positiva) (bits 3-2 - TAEVENT)
	TIMER1->TAMR &= ~(1<<9);						//11-Desabilita interrupão de PWM (bit 9 - TAPWMIE)
	TIMER1->TAILR = SystemCoreClock / freq - 1;			//12-Intervalo de contagem (PWM é setado)
	//Atenção: o match ocorre antes do prescaler!!!
	TIMER1->TAMATCHR = TIMER1->TAILR * duty / 100;	//13-Define quando o PWM será desligado (match)
	//Como o sinal do PWM é invertido a saída será ligada quando o PWM for desligado
	TIMER1->CTL |= 1<<0;		 						//14-Habilita Timer1A e começa a contar (bit 0 - TAEN)
	//NVIC_SetPriority(TIMER1A_IRQn, 5); 	//15-Define a prioridade da interrupção
  //NVIC_EnableIRQ(TIMER1A_IRQn);				//16-Habilita interrupção no NVIC
  //__enable_irq();											//17-Limpa o bit I
}

//*****************************************************************************
//Ajusta GPTM (Timer 1 modo PWM - buzzer do BoosterPack MK II)
//Supondo SystemCoreClock = 16 MHz
//freq mínima = 16.000.000 / 65536 = 244 Hz
//freq máxima = 10 kHz (datasheet buzzer CEM-1203)
//duty em % de 1% em 1% 
void MKII_Timer1_PWM_Set(uint16_t freq, uint16_t duty)	//TODO: o duty cycle está invertido!!!
{
	if((duty > 100) || (freq > 10000))
		return;                        		//Entrada inválida
	
	TIMER1->CTL &= ~(1<<0); 												//1-Desabilita Timer1A durante o setup
	TIMER1->TAILR = SystemCoreClock / freq - 1;			//2-Intervalo de contagem (PWM é setado)
	TIMER1->TAMATCHR = TIMER1->TAILR * duty / 100;	//3-Define quando o PWM será desligado (match)
	//Como o sinal do PWM é invertido a saída será ligada quando o PWM for desligado
	TIMER1->CTL |= 1<<0;		 												//4-Habilita Timer1A e começa a contar (bit 0 - TAEN)
}

//*****************************************************************************
//Configura operação do servo como Timer 3A (PB2)
//Duas etapas:
//1a configurar o GPIO
//2a configurar o Timer como PWM
void MKII_Servo_Init(void)
{
	//1a etapa: configurar o GPIO------------------------------------------------
	//Tivaware
	//Ativa o clock do PORTB
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	//Espera o clock estabilizar
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
	{
	}
	//Seleciona a função de timer para o pino
	//GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2); //GPIO_PORTB_BASE declarado em hw_memmap.h do Tivaware
	GPIOPinTypeTimer(GPIOB_BASE, GPIO_PIN_2);	//GPIOB_BASE declarado em TM4C123GH6PM.h do CMSIS
	//Habilita funcionamento do pino como Timer3
	GPIOPinConfigure(GPIO_PB2_T3CCP0);

//	//CMSIS
//	//Configuração do Port B
//	SYSCTL->RCGCGPIO |= 0x002;     			//1-Ativa o clock do Port B
//  while((SYSCTL->PRGPIO&0x02) == 0);	//2-Aguarda o clock estabilizar
//  GPIOB->AFSEL |= 0x04;				      	//3-Habilita função alternativa no PB2
//  GPIOB->DEN |= 0x04;		        			//4-Habilita I/O difgital no PB2
//  GPIOB->PCTL = (GPIOB->PCTL & 0xFFFFF0FF) + 0x00000700;	//5-Seleciona a função T1CCP0 no PB2
//  GPIOB->AMSEL &= ~0x04;			 	    	//5-Desabilita função analógica no PB2

	//2a etapa: configurar o Timer como PWM--------------------------------------
	//Tivaware
	//Ativa o clock do Timer3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	//Aguarda o clock estabilizar
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3))
	{
	}
	//Desabilita o Timer3A
	TimerDisable(TIMER3_BASE, TIMER_A);	
	//Configura o Timer3 como 2 de 16 bits (A e B), TimerA peródico e PWM
	TimerConfigure(TIMER3_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_A_PWM));
	//Ajusta a saída como invertida (isso porque o contador a decrescente, o match define o ponto para ligar o GPIO)
	TimerControlLevel(TIMER3_BASE, TIMER_A, true);
	//Define o prescale = 5-1 (divide o clock por 5)
	//f=16 MHz, t=62,5 ns, ciclo timer=t*2^16=4,096 ms
	//com prescale de 5: f=3,2 MHz, t=312,5 ns, ciclo timer=20,48 ms
	TimerPrescaleSet(TIMER3_BASE, TIMER_A, 5-1);
	//Ajusta o intervalo (período) para ~20 ms (50 Hz)
	//i=20ms/312,5ns=62208,4
	TimerLoadSet(TIMER3_BASE, TIMER_A, 62208);
	//Ajusta o match (duty cycle)
	//Atenção: o match ocorre antes do prescaler!!!
	//-90 graus=0,6ms -> 9600 ciclos
	//+90 graus=2,4ms -> 38400 ciclos
	//match=a*angulo+b
	//a=(38400-9600)/(90-(-90))=160
	//b=38400-160*90=24000
	//0 graus=24000 ciclos -> 1,5ms
	TimerMatchSet(TIMER3_BASE, TIMER_A, 24000);
	//Habilita o Timer3A
	TimerEnable(TIMER3_BASE, TIMER_A);
	
//	//CMSIS
//	//Configuração do Timer 3 modo PWM
//	SYSCTL->RCGCTIMER |= 0x08;					//1-Habilita o clock do timer
//	while((SYSCTL->PRTIMER&0x08) == 0);	//2-Aguarda o clock estabilizar
//	TIMER3->CTL &= ~(1<<0); 						//3-Desabilita Timer3A durante o setup
//	TIMER3->CFG = 0x04;									//4-Configura o timer para modo 16 bits
//	TIMER3->TAMR |= 1<<3;								//5-Seleciona o modo alternativo PWM (bit 3 - AMS - GPTM Timer A Alternate Mode Select)
//	TIMER3->TAMR &= ~(1<<2);						//6-Desabilita o modo contagem de bordas (bit 2 - CMR - GPTM Timer A Capture Mode)
//	TIMER3->TAMR |= 0x02;								//7-Configura o Timer3A para modo periódico (bits 1-0 - MR - GPTM Timer A Mode)
//	TIMER3->CTL |= 1<<6;								//8-Configura o estado do sinal de PWM (invertido) (bit 6 - TAPWML - GPTM Timer A PWM Output Level)  
//	TIMER3->TAPR = 0x05 - 1;						//9-Configura o valor do prescaler
//	TIMER3->CTL &= ~(3<<2);							//10-Configura o evento de interrupção de PWM (borda positiva) (bits 3-2 - TAEVENT)
//	TIMER3->TAMR &= ~(1<<9);						//11-Desabilita interrupão de PWM (bit 9 - TAPWMIE)
//	TIMER3->TAILR = 62208;							//12-Intervalo de contagem (PWM é setado)
//	TIMER3->TAMATCHR = 24000;						//13-Define quando o PWM será desligado (match)
//	//Como o sinal do PWM é invertido a saída será ligada quando o PWM for desligado
//	TIMER3->CTL |= 1<<0;		 						//14-Habilita Timer1A e começa a contar (bit 0 - TAEN)
//	//NVIC_SetPriority(TIMER1A_IRQn, 5); 	//15-Define a prioridade da interrupção
//  //NVIC_EnableIRQ(TIMER1A_IRQn);				//16-Habilita interrupção no NVIC
//  //__enable_irq();											//17-Limpa o bit I
}

//*****************************************************************************
//Configura operação do servo como Timer 3A
void MKII_Servo_Set_Angle(int angulo)	
{
	int match = 0;
	//Desabilita o timer3A
	TimerDisable(TIMER3_BASE, TIMER_A);	
	//Ajusta o intervalo (período) para ~20 ms (50 Hz)
	//i=20ms/312,5ns=62208,4
	TimerLoadSet(TIMER3_BASE, TIMER_A, 62208);
	//Ajusta o match (duty cycle)
	//Atenção: o match ocorre antes do prescale!!!
	//-90 graus=0,6ms -> 9600 ciclos
	//+90 graus=2,4ms -> 38400 ciclos
	//match=a*angulo+b
	//a=(38400-9600)/(90-(-90))=160
	//b=38400-160*90=24000
	//0 graus=24000 ciclos -> 1,5ms
	match = 160 * angulo + 24000;
	TimerMatchSet(TIMER3_BASE, TIMER_A, match);
	//Habilita o timer3A
	TimerEnable(TIMER3_BASE, TIMER_A);
}

//*****************************************************************************
//	ISRs
//*****************************************************************************
