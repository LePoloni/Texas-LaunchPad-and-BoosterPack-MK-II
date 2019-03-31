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
//	Definições e Apelidos
//*****************************************************************************
#ifndef MKII_H
#define MKII_H
#endif

#define MKII_Buzzer_Init(freq, duty) MKII_Timer1_PWM_Init(freq, duty)
#define MKII_Buzzer_Set(freq, duty) MKII_Timer1_PWM_Set(freq, duty)

//*****************************************************************************
//	Protótipos de Funções
//*****************************************************************************
void MKII_Timer1_PWM_Init(uint16_t freq, uint16_t duty);
void MKII_Timer1_PWM_Set(uint16_t freq, uint16_t duty);
void MKII_Servo_Init(void);
void MKII_Servo_Set_Angle(int angulo);
