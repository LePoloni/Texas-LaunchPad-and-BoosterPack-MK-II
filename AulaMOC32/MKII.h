/******************************************************************************
*	Biblioteca: MK_II
*	Autor: Leandro Poloni Dantas
*	Data: 17/03/19
*	Sobre: Est� biblioteca foi criada com o objetivo de colecionar fun��es
*		     criadas para facilitar a utiliza��o da BoosterPack MK II juntamente
*				 com a LaunchPad TM4C123 da Texas.
*	Vers�o: 1.0
*	Revis�es:
*				26/03/19 - Incluidas as fun��es para controle do Servo Motor pino PB2
*										(J2.19)
*
******************************************************************************/

//*****************************************************************************
//	Defini��es e Apelidos
//*****************************************************************************
#ifndef MKII_H
#define MKII_H
#endif

#define MKII_Buzzer_Init(freq, duty) MKII_Timer1_PWM_Init(freq, duty)
#define MKII_Buzzer_Set(freq, duty) MKII_Timer1_PWM_Set(freq, duty)

//*****************************************************************************
//	Prot�tipos de Fun��es
//*****************************************************************************
void MKII_Timer1_PWM_Init(uint16_t freq, uint16_t duty);
void MKII_Timer1_PWM_Set(uint16_t freq, uint16_t duty);
void MKII_Servo_Init(void);
void MKII_Servo_Set_Angle(int angulo);
