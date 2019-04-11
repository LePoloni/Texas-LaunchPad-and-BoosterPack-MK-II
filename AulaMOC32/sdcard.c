/*
 * sdcard.c
 *
 *  Created on: 15/09/2016
 *  Author: Javier Martínez Arrieta
 *  Version: 1.0
 *  This is part of the sdcard library, with functions that will allow you to read and (in the future) write in an SD card formatted using FAT32 (single partition).
 *	
 *	Revisada: 28/02/2019
 *	Autor: Leandro Poloni Dantas
 *	Versão: 2.0
 *	Nesta revisão foram corrigidos uma série de erros que impediam o uso da biblioteca e novas funções foram implementadas.
 *	Agora é possível ler arquivos no formato TXT e BMP.
 *	Ainda é possível acessar o Master Boot e o Boot Record Information de cada setor.
 *	Foi definida uma função para passagem de funções de print de strings e caracteres.
 *	As funções originais foram renomeadas, Em sua maioria recebeu o prefixo SD_.
 *	Revisada: 01/04/2019
 *	Autor: Leandro Poloni Dantas
 *	Versão: 2.1
 *  Habilitado o pull-up do sinal MISO
 *	A função SD_list_dirs_and_files foi alterada na tentativa de tornar mais rápida a busca por arquivos
 *	A função SD_initialize_sd tenha seu tipo de retorno alterado de void para unsigned char
 */

/* Part of this example (partially modified functions rcvr_datablock, rcvr_spi_m, disk_timerproc, Timer5A_Handler, Timer5_Init, is_ready, send_command and part of initialize_sd) accompanies the books
   Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers, Volume 3,
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2013

   Volume 3, Program 6.3, section 6.6   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file (concretely the functions mentioned)
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "sdcard.h"
#include "inc/tm4c123gh6pm.h"
#include <stdio.h>

//#define  depura
#define SSI2_com_MKII


unsigned char Timer1,Timer2;
unsigned long lba_begin_address,number_of_sectors,lba_addr,cluster_start,file_size,cluster_begin_lba,fat_begin_lba,sectors_per_fat,root_dir_first_cluster;
unsigned long previous_cluster=0,cluster_dir=0;
unsigned char sectors_per_cluster;
char cuenta=0,current_count=0;
char finish=0;
int row=0,column=0,number=0;
//Leandro (29/02/19): Define um ponteiro para funções do tipo TipoFuncao
TipoFuncaoString *prints;
TipoFuncaoChar *printc;

typedef struct
{
	char hour;
	char minute;
	unsigned int year;
	char month;
	char day;
	long size;
	long first_cluster;
}tfile_info;

typedef struct
{
	tfile_info info;
	unsigned char file_dir_name[255];
}tfile_name;

typedef struct
{
	tfile_name name;
	enum type_of_file
	{
		IS_NONE,
		IS_DIR,
		IS_FILE
	}type;
}tfile_dir;

tfile_dir file_dir[40];

/**
 * Writes to the SD card
 */
void sd_write(char message,enum SSI SSI_number)
{
	unsigned short volatile rcvdata;
	switch(SSI_number)
	{
		case SSI_0:
		{
			// wait until transmit FIFO not full
			while((SSI0_SR_R&SSI_SR_TNF)==0){};
			//DC = DC_DATA;
			SSI0_DR_R = message;
			while((SSI0_SR_R&SSI_SR_RNE)==0){};
			rcvdata = SSI0_DR_R;
			break;
		}
		case SSI_1:
		{
			while((SSI1_SR_R&SSI_SR_TNF)==0){};
			//DC = DC_DATA;
			SSI1_DR_R = message;
			while((SSI1_SR_R&SSI_SR_RNE)==0){};
			rcvdata = SSI1_DR_R;
			break;
		}
		case SSI_2:
		{
			while((SSI2_SR_R&SSI_SR_TNF)==0){};
			//DC = DC_DATA;
			SSI2_DR_R = message;
			while((SSI2_SR_R&SSI_SR_RNE)==0){};
			rcvdata = SSI2_DR_R;
			break;
		}
		case SSI_3:
		{
			while((SSI1_SR_R&SSI_SR_TNF)==0){};
			//DC = DC_DATA;
			SSI3_DR_R = message;
			while((SSI3_SR_R&SSI_SR_RNE)==0){};
			rcvdata = SSI3_DR_R;
			break;
		}
	}
}

/*
 * Reads from the SD card
 */
unsigned char sd_read(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			while((SSI0_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
			SSI0_DR_R = 0xFF;                      // data out, garbage
			while((SSI0_SR_R&SSI_SR_RNE)==0){};    // wait until response
			return (unsigned char) SSI0_DR_R;      // read received data
			break;
		}
		case SSI_1:
		{
			while((SSI1_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
			SSI1_DR_R = 0xFF;                      // data out, garbage
			while((SSI1_SR_R&SSI_SR_RNE)==0){};    // wait until response
			return (unsigned char) SSI1_DR_R;      // read received data
			break;
		}
		case SSI_2:
		{
			while((SSI2_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
			SSI2_DR_R = 0xFF;                      // data out, garbage
			while((SSI2_SR_R&SSI_SR_RNE)==0){};    // wait until response
			return (unsigned char) SSI2_DR_R;      // read received data
			break;
		}
		case SSI_3:
		{
			while((SSI3_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
			SSI3_DR_R = 0xFF;                      // data out, garbage
			while((SSI3_SR_R&SSI_SR_RNE)==0){};    // wait until response
			return (unsigned char) SSI3_DR_R;      // read received data
			break;
		}
	}
}

/*
 * Wait until sd card is ready
 */
unsigned char is_ready(enum SSI SSI_number){
  unsigned char response;
  Timer2 = 50;    /* Wait for ready in timeout of 500ms */
  //sd_write(DATA,0xFF,SSI0);
  do
  {
    response = sd_read(SSI_number);
	}while ((response != 0xFF) && Timer2);
  return response;
}


/*
 * Sends the command, preparing the packet to be sent
 */
unsigned char send_command(unsigned char command, unsigned long argument, enum SSI SSI_number)
{
	/* Argument */
	unsigned char crc, response,n;
	if (is_ready(SSI_number) != 0xFF) return 0xFF;

#ifdef depura
//prints("ready");
#endif
	
  /* Send command packet */
	sd_write(command,SSI_number);                        /* Command */
	sd_write((unsigned char)(argument >> 24),SSI_number);        /* Argument[31..24] */
	sd_write((unsigned char)(argument >> 16),SSI_number);        /* Argument[23..16] */
	sd_write((unsigned char)(argument >> 8),SSI_number);            /* Argument[15..8] */
	sd_write((unsigned char)argument,SSI_number);                /* Argument[7..0] */
	crc = 0;
	if (command == CMD0)
	{
		crc = 0x95;            /* CRC for CMD0(0) */
	}
	if (command == CMD8)
	{
		crc = 0x87;            /* CRC for CMD8(0x1AA) */
	}
	sd_write(crc,SSI_number);

  /* Receive command response */
	if (command == CMD12) sd_write(0xFF,SSI_number);        /* Skip a stuff byte when stop reading */
	n = 255;                                /* Wait for a valid response in timeout of 10 attempts */
	do
	{
		response = sd_read(SSI_number);
	}while ((response & 0x80) && --n);
	return response;
}

/*
 * Initializes the SD card
 */
unsigned char SD_initialize_sd(enum SSI SSI_number)
{
	unsigned char i,j;
	unsigned char ocr[4];
	unsigned char sd_type;
	//Sends a 1 through CS and MOSI lines for at least 74 clock cycles
	SD_cs_high(SSI_number);
	dummy_clock(SSI_number);
	SD_cs_low(SSI_number);

#ifdef depura	
//	prints("CMD0 = ");
//	printc(send_command(CMD0, 0,SSI_number) + '0');
//	printEnter();
//	while(1);	//teste
//	j = send_command(CMD0, 0,SSI_number);
//	printc(j + '0');
//	printEnter();
//	while(1);	//teste	
#endif

	i=0;
	
	/*Checks if SD card is in IDLE mode. If so, response will be 1*/
	//Leandro (20/02/2019) - mudei a forma de testar se o SD está ocioso -> n tentativa até o erro
	//if(send_command(CMD0, 0,SSI_number) == 1)
	//{	
	while(((j=send_command(CMD0, 0,SSI_number))!=1)  && ((i++)<100))
	{
//#ifdef depura		
		prints(".");
//#endif
	}
	i=0;
	if(j==1)
	{	
#ifdef depura
prints("r2");
#endif
		Timer1 = 100; /* Initialization timeout of 1000 msec */
		//Leandro (comentário): Send the CMD8 command next to check the version of your SD card
		if(send_command(CMD8, 0x1AA, SSI_number) == 1)
		{
#ifdef depura
prints("r3");
#endif
			/* SDC Ver2+ */
			for(i=0;i<4;i++)
			{
				ocr[i]=sd_read(SSI_number);
			}
			if(ocr[2]==0x01&&ocr[3]==0xAA)
			{
				//sends ACMD41, which is a command sequence of CMD55 and CMD41
				do
				{
					if(send_command(CMD55, 0, SSI_number) <= 1 && send_command(CMD41, 1UL << 30,SSI_number) == 0)
					{
						break; //R1 response is 0x00
					}
				}while(Timer1);
				//Leandro (comentário): Just to ensure whether the SD Card is functioning in the correct working voltage, send the CMD58 Command
				if(Timer1 && send_command(CMD58, 0,SSI_number) == 0)
				{
					for(i=0;i<4;i++)
					{
						ocr[i]=sd_read(SSI_number);
						sd_type = (ocr[0] & 0x40) ? 6 : 2;	//Leandro (comentário): Se o bit 30 é 1, é um cartão SDHC
					}
				}
			}
		}
		else
		{
			/*It is not SD version 2 or upper*/
			sd_type=(send_command(CMD55, 0,SSI_number)<=1 && send_command(CMD41, 0,SSI_number) <= 1) ? 2 : 1;    /*Check if SD or MMC*/
			do
			{
				if(sd_type==2)
				{
					if(send_command(CMD55, 0,SSI_number)<=1 && send_command(CMD41, 0,SSI_number)==0) /*ACMD41*/
			    {
						break;
			    }
				}
				else
				{
					if (send_command(CMD1, 0,SSI_number) == 0) /*CMD1*/
					{
						break;
					}
			  }
			}while(Timer1);
			if(!Timer1 || send_command(CMD16, 512,SSI_number) != 0)    /*Select R/W block length if timeput not reached*/
			{
				sd_type=0;
			}
		}
	}
	else
	{
		//Leandro (28/02/2019) - ajuste de compatibilidade
		prints("Failure in CMD0");
		return 1;
	}
	return 0;
}


void startSSI0()
{
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000001;   	  //  activate clock for Port A
	SYSCTL_RCGCSSI_R|=SYSCTL_RCGCSSI_R0;		  		  //  activate clock for SSI0
	delay = SYSCTL_RCGC2_R;         	  //  allow time for clock to stabilize
	GPIO_PORTA_DIR_R |= 0x08;             // make PA6,7 out
	GPIO_PORTA_DR4R_R |= 0xEC;            // 4mA output on outputs
	GPIO_PORTA_AFSEL_R |= 0x34;           // enable alt funct on PA2,4,5
	GPIO_PORTA_AFSEL_R &= ~0xC8;          // disable alt funct on PA3,6,7
	GPIO_PORTA_DEN_R |= 0xFC;             // enable digital I/O on PA2,3,4,5,6,7
										  // configure PA2,4,5 as SSI
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF00F0FF)+0x00220200;
	                                      // configure PA6,7 as GPIO
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0x00FFFFFF)+0x00000000;
	GPIO_PORTA_AMSEL_R &= ~0xFC;          // disable analog functionality on PA2,3,4,5
	SSI0_CR1_R&=~SSI_CR1_SSE;		  	  // Disable SSI while configuring it
	SSI0_CR1_R&=~SSI_CR1_MS;		      // Set as Master
	SSI0_CC_R|=SSI_CC_CS_M; 			  // Configure clock source
	SSI0_CC_R|=SSI_CC_CS_SYSPLL; 		  // Configure clock source
	SSI0_CC_R|=SSI_CPSR_CPSDVSR_M;		  // Configure prescale divisor
	SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+10; // must be even number
	SSI0_CR0_R |=0x0300;
	SSI0_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);
	SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
	                                        // DSS = 8-bit data
	SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
	SSI0_CR1_R|=SSI_CR1_SSE;		  // 3)Enable SSI
}

void startSSI1()
{
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x30;   		// activate clock for Port E and Port F
	SYSCTL_RCGCSSI_R|=SYSCTL_RCGCSSI_R1;	// activate clock for SSI1
	delay = SYSCTL_RCGC2_R;         		// allow time for clock to stabilize
	GPIO_PORTF_LOCK_R=0x4C4F434B;
	GPIO_PORTF_CR_R=0x01;
	GPIO_PORTF_DIR_R |= 0x08;             // make PF3 out
	GPIO_PORTF_AFSEL_R |= 0x07;           // enable alt funct on PF0, PF1 and PF2
	GPIO_PORTF_AFSEL_R &= ~0xF8;          // disable alt funct on PF3
	GPIO_PORTF_DEN_R |= 0x0F;             // enable digital I/O on PF0,PF1,PF2,PF3
                                          // configure PF0, PF1 and PF2 as SSI
	GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0x00000FFF)+0x00000222;
	                                      // configure PF3 as GPIO
	GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF0FFF)+0x00000000;
	GPIO_PORTF_AMSEL_R &= ~0x0F;          // disable analog functionality on PF0,PF1,PF2,PF3
	SSI1_CR1_R&=~SSI_CR1_SSE;		  		// Disable SSI while configuring it
	SSI1_CR1_R&=~SSI_CR1_MS;		  		// Set as Master
	SSI1_CC_R|=SSI_CC_CS_M; 				// Configure clock source
	SSI1_CC_R|=SSI_CC_CS_SYSPLL; 			// Configure clock source
	SSI1_CC_R|=SSI_CPSR_CPSDVSR_M;		// Configure prescale divisor
	SSI1_CPSR_R = (SSI1_CPSR_R&~SSI_CPSR_CPSDVSR_M)+10; // must be even number
	SSI1_CR0_R |=0x0300;
	SSI1_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);
	SSI1_CR0_R = (SSI1_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
	                                        // DSS = 8-bit data
	SSI1_CR0_R = (SSI1_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
	SSI1_CR1_R|=SSI_CR1_SSE;		  		// Enable SSI
}

void SD_startSSI2()
{
	volatile unsigned long delay;
	//SYSCTL_RCGC2_R |= 0x00000002;   			// activate clock for Port B
	//Leandro (17/02/2019) - atualização de registrador RCGC2 -> RCGCGPIO
	SYSCTL_RCGCGPIO_R |= 0x00000002;   		// activate clock for Port B
	SYSCTL_RCGCSSI_R|=SYSCTL_RCGCSSI_R2;	// activate clock for SSI2
	delay = SYSCTL_RCGC2_R;         			// allow time for clock to stabilize
	GPIO_PORTB_LOCK_R=0x4C4F434B;
	GPIO_PORTB_CR_R=0xF0;
#ifndef SSI2_com_MKII
	GPIO_PORTB_DIR_R |= 0x20;             // make PB5 out
#endif
	GPIO_PORTB_DIR_R |= 0x80;             // make PB7 out
	GPIO_PORTB_AFSEL_R |= 0xD0;           // enable alt funct on PB4,PB6 and PB7
	GPIO_PORTB_DEN_R |= 0xD0;             // enable digital I/O on PB4,PB5,PB6 and PB7
#ifndef SSI2_com_MKII
	GPIO_PORTB_DEN_R |= 0x20;             // enable digital I/O on PB5
#endif	
																				// configure PB4,PB6 and PB7 as SSI
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x00F0FFFF)+0x22020000;
#ifndef SSI2_com_MKII
	                                      // configure PB5 as GPIO
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFF0FFFFF)+0x00000000;
#endif
	GPIO_PORTB_AMSEL_R &= ~0xD0;          // disable analog functionality from PB4, PB6 and PB7
#ifndef SSI2_com_MKII
	GPIO_PORTB_AMSEL_R &= ~0x20;          // disable analog functionality from PB5
#endif
	//Leandro (19/02/2019) - Pull-up no clock
	GPIO_PORTB_PUR_R |= 0x10;             // enable pull-up on PB4
	//Leandro (01/04/2019) - Pull-up no MISO
	GPIO_PORTB_PUR_R |= 0x40;             // enable pull-up on PB6
	
	SSI2_CR1_R &= ~SSI_CR1_SSE;		  			// Disable SSI while configuring it
	SSI2_CR1_R &= ~SSI_CR1_MS;		  			// Set as Master
	//Leandro (19/02/2019) - Correção na máscara
	//SSI2_CC_R|=SSI_CC_CS_M; 						// Configure clock source
	SSI2_CC_R &= ~(SSI_CC_CS_M); 					// Configure clock source
	SSI2_CC_R |= SSI_CC_CS_SYSPLL; 				// Configure clock source -> System clock
	//Leandro (19/02/2019) - Correção no prescale
	//SSI2_CC_R|=SSI_CPSR_CPSDVSR_M;			// Configure prescale divisor
	SSI2_CPSR_R = (SSI2_CPSR_R & ~SSI_CPSR_CPSDVSR_M)+10; // must be even number -> 10
	SSI2_CR0_R |=0x0300;									// SSI Serial Clock Rate	 (BR=SysClk/(CPSDVSR * (1 + SCR)))
																				//														 16MHz /(10      * (1 + 3  )) = 400 kHz
	//Leandro (26/02/2019) - the MultiMediaCard powers up in the open-drain mode and cannot handle 
	//a clock faster than 400 Khz. Once the MultiMediaCard completes the initialization process, 
	//the card switches to the push-pull mode. In the push-pull mode the MultiMediaCard can run at the maximum clock speed.
	SSI2_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);	//SSI Serial Clock Phase e SSI Serial Clock Polarity
	SSI2_CR0_R = (SSI2_CR0_R & ~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;	//SSI Frame Format Select -> Freescale SPI Frame Format
	SSI2_CR0_R = (SSI2_CR0_R & ~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;			// DSS = 8-bit data
	SSI2_CR1_R |= SSI_CR1_SSE;		  			// 3)Enable SSI
	
#ifdef SSI2_com_MKII
	//CS passa do pino PB5 para o pino PE0
	//1 - Ativar o clock (RCGCGPIO, RCGC2 só para compatibilidade)
	SYSCTL_RCGCGPIO_R |= 0x00000010;
	//2 - Verificar se o port está pronto (PRGPIO)
	while(!(SYSCTL_PRGPIO_R & 0x10));
	//3 - Destravar o port (LOCK e CR)
	GPIO_PORTE_LOCK_R = 0x4C4F434B;
	GPIO_PORTE_CR_R |= 0x01;
	//4 - Desabilitar função analógica (AMSEL)
	GPIO_PORTE_AMSEL_R &= 0x01;
	//5 - Selecionar a função dos pinos (PCTL)
	GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFFFFF0)+0x00000000;
	//6 - Definir o sentido dos pinos (DIR)
	GPIO_PORTE_DIR_R |= 0x01;
	//7 - Desabilitar funções alternativas (AFSEL)
	GPIO_PORTE_AFSEL_R &= ~0x01;
	//8 - Desabilitar pull-ups (PUR)
	GPIO_PORTE_PUR_R &= ~0x01;
	//9 - Habilitar função digital (DEN)
	GPIO_PORTE_DEN_R |= 0x01;
	//10 - Iniciar ligado
	GPIO_PORTE_DATA_R |= 0x01;
#endif
}

void startSSI3()
{
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000008;   		// activate clock for Port D
	SYSCTL_RCGCSSI_R|=SYSCTL_RCGCSSI_R3;	// activate clock for SSI3
	delay = SYSCTL_RCGC2_R;         		// allow time for clock to stabilize
	GPIO_PORTD_LOCK_R=0x4C4F434B;
	GPIO_PORTD_CR_R=0x80;
	GPIO_PORTD_DIR_R |= 0x08;             // make PD3 out
	GPIO_PORTD_AFSEL_R |= 0x0D;           // enable alt funct on PD0, PD2 and PD3
	GPIO_PORTD_AFSEL_R &= ~0xF2;          // disable alt funct on PD1
	GPIO_PORTD_DEN_R |= 0x0F;             // enable digital I/O on PD0, PD1, PD2 and PD3
	// configure PD0, PD2 and PD3 as SSI
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF00F0)+0x00001101;
	// configure PD1 as GPIO
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFFFFF0F)+0x00000000;
	                                        // configure PD0, PD1 and PD3 as SSI
	GPIO_PORTD_AMSEL_R &= ~0xCF;          // disable analog functionality on PD0, PD1, PD2, PD3, PD6 and PD7
	SSI3_CR1_R&=~SSI_CR1_SSE;		  		// Disable SSI while configuring it
	SSI3_CR1_R&=~SSI_CR1_MS;		  		// Set as Master
	SSI3_CC_R|=SSI_CC_CS_M; 				// Configure clock source
	SSI3_CC_R|=SSI_CC_CS_SYSPLL; 			// Configure clock source
	SSI3_CC_R|=SSI_CPSR_CPSDVSR_M;		// Configure prescale divisor
	SSI3_CPSR_R = (SSI3_CPSR_R&~SSI_CPSR_CPSDVSR_M)+10; // must be even number
	SSI3_CR0_R |=0x0300;
	SSI3_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);
	SSI3_CR0_R = (SSI3_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
	                                        // DSS = 8-bit data
	SSI3_CR0_R = (SSI3_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
	SSI3_CR1_R|=SSI_CR1_SSE;		  // 3)Enable SSI
}

/*
 * Makes use of the clock along with CS and MOSI to make the SD card readable using SPI
 */
void dummy_clock(enum SSI SSI_number)
{
	unsigned int i;
	//In order to initialize and set SPI mode, there should be at least 74 clock cycles with MOSI and CS set to 1
	for ( i = 0; i < 2; i++);
	//CS set high
	SD_cs_high(SSI_number);
	//Disables SSI on TX/MOSI pin to send a 1
	tx_high(SSI_number);				//Leandro (comentário): não é necessário forçar 1 no pino, basta enviar 0xFF
	for ( i = 0; i < 10; i++)
	{
		sd_write(0xFF,SSI_number);
	}
	tx_SSI(SSI_number);
}

/*
 * Gets the first cluster of a file or directory
 */
long SD_get_first_cluster(int pos)
{
	return file_dir[pos].name.info.first_cluster;
}

/*
 * Gets the first cluster of the root directory
 */
long SD_get_root_dir_first_cluster(void)
{
	return root_dir_first_cluster;
}

/*
 * Makes chip select line high
 */
void SD_cs_high(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			GPIO_PORTA_DATA_R|=0x08;
			break;
		}
		case SSI_1:
		{
			GPIO_PORTF_DATA_R|=0x08;
			break;
		}
		case SSI_2:
		{
#ifndef SSI2_com_MKII
			GPIO_PORTB_DATA_R |= 0x20;
#endif
#ifdef SSI2_com_MKII
			GPIO_PORTE_DATA_R |= 0x01;
#endif
			break;
		}
		case SSI_3:
		{
			GPIO_PORTD_DATA_R|=0x02;
			break;
		}
	}
}

/*
 * Makes chip select line low
 */
void SD_cs_low(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			GPIO_PORTA_DATA_R&=~0x08;
			break;
		}
		case SSI_1:
		{
			GPIO_PORTF_DATA_R&=~0x08;
			break;
		}
		case SSI_2:
		{
#ifndef SSI2_com_MKII
			GPIO_PORTB_DATA_R &= ~0x20;
#endif
#ifdef SSI2_com_MKII
			GPIO_PORTE_DATA_R &= ~0x01;
#endif
			break;
		}
		case SSI_3:
		{
			GPIO_PORTD_DATA_R&=~0x02;
			break;
		}
	}
}

/*
 * Writes a '1' in the transmission line of the SSI that is being used
 */
void tx_high(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			GPIO_PORTA_AFSEL_R &= ~0x20;           // disable alt funct on PA5
			GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0FFFFF);
			GPIO_PORTA_DATA_R |= 0x20;            // PA5 high
			break;
		}
		case SSI_1:
		{
			GPIO_PORTF_AFSEL_R &= ~0x02;           // disable alt funct on PF1
			GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFF0F);
			GPIO_PORTF_DATA_R |= 0x02;            // PF1 high
			break;
		}
		case SSI_2:
		{
			GPIO_PORTB_AFSEL_R &= ~0x80;           // disable alt funct on PB7
			GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0FFFFFFF);
			//Leandro (20/02/2019) - O pino não foi definido como saída, falta a linha abaixo
			//GPIO_PORTB_DIR_R |= 0x80;             // PB7 output (04/03/2019 - passou a ser definido na inicialização)
			
			GPIO_PORTB_DATA_R |= 0x80;            // PB7 high
			break;
		}
		case SSI_3:
		{
			GPIO_PORTD_AFSEL_R &= ~0x08;           // disable alt funct on PD3
			GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0FFF);
			GPIO_PORTD_DATA_R |= 0x08;            // PD3 high
			break;
		}
	}
}

/*
 * Configure again the transmission line of the SSI that is being used
 */
void tx_SSI(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			GPIO_PORTA_AFSEL_R |= 0x20;           // enable alt funct on PA5
			GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0FFFFF)+0x00200000;
			break;
		}
		case SSI_1:
		{
			GPIO_PORTF_AFSEL_R |= 0x02;           // enable alt funct on PF1
			GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFF0F)+0x00000020;
			break;
		}
		case SSI_2:
		{
			GPIO_PORTB_AFSEL_R |= 0x80;           // enable alt funct on PB7
			GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0FFFFFFF)+0x20000000;
			break;
		}
		case SSI_3:
		{
			GPIO_PORTD_AFSEL_R |= 0x08;           // enable alt funct on PD3
			GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0FFF)+0x00001000;
			break;
		}
	}
}

/*Change speed to (?) 8 MHz*/
void SD_change_speed(enum SSI SSI_number)
{
	switch(SSI_number)
	{
		case SSI_0:
		{
			SSI0_CC_R|=SSI_CPSR_CPSDVSR_M;// Configure prescale divisor
			SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+2; // must be even number
			SSI0_CR0_R |=0x0000;
			break;
		}
		case SSI_1:
		{
			SSI1_CC_R|=SSI_CPSR_CPSDVSR_M;// Configure prescale divisor
			SSI1_CPSR_R = (SSI1_CPSR_R&~SSI_CPSR_CPSDVSR_M)+2; // must be even number
			SSI1_CR0_R |=0x0000;
			break;
		}
		case SSI_2:
		{
			//Leandro (26/02/2019) - Correção no prescale
			//SSI2_CC_R |= SSI_CPSR_CPSDVSR_M;	// Configure prescale divisor
			//SSI2_CPSR_R = (SSI2_CPSR_R & ~SSI_CPSR_CPSDVSR_M)+2; // must be even number
			//SSI2_CR0_R |= 0x0000;
			
			SSI2_CC_R &= ~(SSI_CC_CS_M); 					// Configure clock source
			SSI2_CC_R |= SSI_CC_CS_SYSPLL; 				// Configure clock source -> System clock
			SSI2_CPSR_R = (SSI2_CPSR_R & ~SSI_CPSR_CPSDVSR_M)+2; // must be even number -> 2
			SSI2_CR0_R |=0x0000;									// SSI Serial Clock Rate	 (BR=SysClk/(CPSDVSR * (1 + SCR)))
																						//														 16MHz /(2       * (1 + 0  )) = 8 MHz
			break;
		}
		case SSI_3:
		{
			SSI3_CC_R|=SSI_CPSR_CPSDVSR_M;// Configure prescale divisor
			SSI3_CPSR_R = (SSI3_CPSR_R&~SSI_CPSR_CPSDVSR_M)+2; // must be even number
			SSI3_CR0_R |=0x0000;
			break;
		}
	}
}

void read_csd(enum SSI SSI_number)
{
	unsigned char csd[16];
	send_command(CMD9,0,SSI_number);
	rcvr_datablock(csd,16,SSI_number);
}

/*
 * Verify if file system is FAT32
 */
void SD_read_first_sector(enum SSI SSI_number)
{
	unsigned char buffer[512];
	if (send_command(CMD17, 0x00000000,SSI_number) == 0)
	{
		rcvr_datablock(buffer, 512,SSI_number);
	}
	if((buffer[450] == 0x0B || buffer[450] == 0x0C) && buffer[510] == 0x55 && buffer[511] == 0xAA)
	{
		//Leandro (28/02/2019) - ajuste de compatibilidade
		printc("FS is FAT32"); printEnter();
	}
	else
	{
		//Leandro (28/02/2019) - ajuste de compatibilidade
		printc("Error FAT32"); printEnter();		
	}
	//454 = 1BE + 08 -> Number of Sectors Betweenthe MBR and the First Sector in the Partition	
	lba_begin_address=(unsigned long)buffer[454]+(((unsigned long)buffer[455])<<8)+(((unsigned long)buffer[456])<<16)+(((unsigned long)buffer[457])<<24);
	//458 = 1BE + 0C -> Number of Sectors in the Partition
	number_of_sectors=(unsigned long)buffer[458]+(((unsigned long)buffer[459])<<8)+(((unsigned long)buffer[460])<<16)+(((unsigned long)buffer[461])<<24);
	
	//Leandro (26/02/2019): Função criada para depuração
	show_MBR(buffer);		//Mostra todo conteúdo do Master Boot Record (1o setor do SDCard não particionado)
}

/*
 * Reads the necessary data so as to be able to access the files and directories
 */
void SD_read_disk_data(enum SSI SSI_number)
{
	unsigned char buffer[512];
	if (send_command(CMD17, lba_begin_address,SSI_number) == 0)
	{
		rcvr_datablock(buffer, 512,SSI_number);
	}
	fat_begin_lba = lba_begin_address + (unsigned long)buffer[14] + (((unsigned long)buffer[15])<<8); //Partition_LBA_BEGIN + Number of reserved sectors
	sectors_per_fat=((unsigned long)buffer[36]+(((unsigned long)buffer[37])<<8)+(((unsigned long)buffer[38])<<16)+(((unsigned long)buffer[39])<<24));
	cluster_begin_lba = fat_begin_lba + ((unsigned long)buffer[16] * ((unsigned long)buffer[36]+(((unsigned long)buffer[37])<<8)+(((unsigned long)buffer[38])<<16)+(((unsigned long)buffer[39])<<24)));//Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	sectors_per_cluster = (unsigned char) buffer[13];//BPB_SecPerClus;
	root_dir_first_cluster = (unsigned long)buffer[44]+(((unsigned long)buffer[45])<<8)+(((unsigned long)buffer[46])<<16)+(((unsigned long)buffer[47])<<24);//BPB_RootClus;
	lba_addr = cluster_begin_lba + ((root_dir_first_cluster/*cluster_number*/ - 2) * (unsigned long)sectors_per_cluster);

	//Leandro (26/02/2019): Função criada para depuração
	show_MBR(buffer);		//Mostra todo conteúdo do Master Boot Record (1o setor do SDCard não particionado)
}

/*
 * List directories and files using the long name (if it has) or the short name, listing subdirectories as well if asked by the user
 */
long SD_list_dirs_and_files(long next_cluster,enum name_type name, enum get_subdirs subdirs, enum SSI SSI_number)
{
	unsigned char buffer[512];
	int position=0,filename_position=0;
	int n=0;
	unsigned long count=10,sector=0,sectors_to_be_read=sectors_per_cluster;//Calculate this
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	//Leandro (27/02/2019): Ajuste para compatibilidade
	char valor[30];	
	
	if(cluster_dir == next_cluster)
	{
		cluster_dir=0;
	}
	if(send_command(CMD18,address,SSI_number)==0)	//CMD18 - Leitura de múltiplos blocos
	{
		do
		{
			rcvr_datablock(buffer, 512,SSI_number);	//Lê um bloco
			do
			{
				if(position<512 && filename_position<255 /*&& position%32==0 && buffer[position]!=0x00 && buffer[position]!=0x05 /*&& buffer[position]!=0x00*/)
				{//Long filename text - 11th byte is 0x0F
					if(position%32==0)
					{//Check if file has a long filename text, normal record with short filename, unused or end of a directory
						
						//Leandro (01/04/2019): Informação compplementar	
						//If DIR_Name[0] == 0xE5, then the directory entry is free (there is no file or directory name in this entry).
						//If DIR_Name[0] == 0x00, then the directory entry is free (same as for 0xE5), and there are no
						//allocated directory entries after this one (all of the DIR_Name[0] bytes in all of the entries after
						//this one are also set to 0).
						//The special 0 value, rather than the 0xE5 value, indicates to FAT file system driver code that the
						//rest of the entries in this directory do not need to be examined because they are all free.
						//The first directory entry has DIR_Name set to (o prórpio diretório):
						//".       "
						//The second has DIR_Name set to (o diretório pai):
						//"..      "
						
						if(buffer[position]==0x00 || buffer[position]==0x2E)	//0x00 = diretório vazio, 2E = '.' = o próprio diretório
						{//End of directory
							position=position+32;
							//Leandro (01/04/2019): Tentativa de acelerar a leitura do cartão
							//Quando o nome do arquivo começa com 0x00 é sinal que não há mais nada no diretório
							if(buffer[position]==0x00)
							{
								position=512;	//Faz saltar para o próximo setor
							}
						}
						else
						{
							if(buffer[position]==0xE5)
							{//Deleted file or directory that is maintained until overriden
								position=position+32;
							}
							else
							{
								if(buffer[position+11]==0x0F && name==LONG_NAME)//Review this
								{//Long filename text (to be ignored?)
									//Get the number of groups of 32 bytes corresponding to the name of file or directory
									short keep_counting=1,do_not_continue=0,is_dir=0;
									int pos=position+32;
									do
									{
										if(buffer[pos+11]==0x0F)
										{
											pos=pos+32;
										}
										else
										{
											//Is it hidden, operating system or Volume ID?
											if((buffer[pos+11]&0x0E)>0x00)
											{
												do_not_continue=1;
											}
											else
											{
												if((buffer[pos+11]&0x10)==0x10)
												{
													is_dir=1;
												}
											}
											keep_counting=0;
										}
									}while(keep_counting==1);
									if(do_not_continue==0)
									{
										int num_blocks=(pos-position)/32;
										int current_block=0;
										if(is_dir)
										{
											file_dir[cuenta].type=IS_DIR;
										}
										else
										{
											file_dir[cuenta].type=IS_FILE;
										}
										//Get sequence number
										do
										{
											int seq_num=buffer[position]&0x1F;
											filename_position=32*(seq_num-1);
											position++;
											for(n=1;n<32;n++)
											{
												if((n<11 || n>13) && n!=26)
												{
													file_dir[cuenta].name.file_dir_name[filename_position+n]=buffer[position];
												}
												position++;
											}
											current_block++;
											num_blocks--;
										}while(num_blocks>0);
										clean_name();
										int time=(((int)(buffer[position/*-11*/+23]))<<8) + ((int)buffer[position/*-11*/+22]);
										int date=(((int)(buffer[position/*-11*/+25]))<<8) + ((int)buffer[position/*-11*/+24]);
										file_dir[cuenta].name.info.minute=(time&0x07E0)>>5;
										file_dir[cuenta].name.info.hour=(time&0xF800)>>11;
										file_dir[cuenta].name.info.month=((date&0x01E0)>>5);
										file_dir[cuenta].name.info.year=((date&0xFE00)>>9)+1980;
										file_dir[cuenta].name.info.day=date&0x001F;
										file_dir[cuenta].name.info.size=(long)((buffer[position/*-11*/+31])<<24)+(long)((buffer[position/*-11*/+30])<<16)+(long)((buffer[position/*-11*/+29])<<8)+(long)(buffer[position/*-11*/+28]);
										file_dir[cuenta].name.info.first_cluster=(long)((buffer[position/*-11*/+21])<<24)+(long)((buffer[position/*-11*/+20])<<16)+(long)((buffer[position/*-11*/+27])<<8)+(long)(buffer[position/*-11*/+26]);
										position=position+32;
									}
									else
									{
										position=position+(32*(pos/32));
									}
								}
								else
								{//Normal record with short filename
									//Is it a directory (not system's)?
									if((buffer[position+11]&0x30)==0x10 && (buffer[position+11]&0x0E)==0x00)
									{
										file_dir[cuenta].type=IS_DIR;
										for(n=0;n<11;n++)
										{
											file_dir[cuenta].name.file_dir_name[n]=buffer[position];
											position++;
										}
										int time=(((int)(buffer[position-11+23]))<<8) + ((int)buffer[position-11+22]);
										int date=(((int)(buffer[position-11+25]))<<8) + ((int)buffer[position-11+24]);
										file_dir[cuenta].name.info.minute=(time&0x07E0)>>5;
										file_dir[cuenta].name.info.hour=(time&0xF800)>>11;
										file_dir[cuenta].name.info.month=((date&0x01E0)>>5);
										file_dir[cuenta].name.info.year=((date&0xFE00)>>9)+1980;
										file_dir[cuenta].name.info.day=date&0x001F;
										file_dir[cuenta].name.info.size=(long)((buffer[position-11+31])<<24)+(long)((buffer[position-11+30])<<16)+(long)((buffer[position-11+29])<<8)+(long)(buffer[position-11+28]);
										file_dir[cuenta].name.info.first_cluster=(long)((buffer[position-11+21])<<24)+(long)((buffer[position-11+20])<<16)+(long)((buffer[position-11+27])<<8)+(long)(buffer[position-11+26]);
									}
									else
									{
										if(((buffer[position+11])&0x30)==0x20 && ((buffer[position+11])&0x0E)==0x00)
										{
											file_dir[cuenta].type=IS_FILE;
											for(n=0;n<11;n++)
											{
												file_dir[cuenta].name.file_dir_name[n]=buffer[position];
												position++;
											}
											int time=(((int)(buffer[position-11+23]))<<8) + ((int)buffer[position-11+22]);
											int date=(((int)(buffer[position-11+25]))<<8) + ((int)buffer[position-11+24]);
											file_dir[cuenta].name.info.minute=(time&0x07E0)>>5;
											file_dir[cuenta].name.info.hour=(time&0xF800)>>11;
											file_dir[cuenta].name.info.month=((date&0x01E0)>>5);
											file_dir[cuenta].name.info.year=((date&0xFE00)>>9)+1980;
											file_dir[cuenta].name.info.day=date&0x001F;
											file_dir[cuenta].name.info.size=(long)((buffer[position-11+31])<<24)+(long)((buffer[position-11+30])<<16)+(long)((buffer[position-11+29])<<8)+(long)(buffer[position-11+28]);
											file_dir[cuenta].name.info.first_cluster=(long)((buffer[position-11+21])<<24)+(long)((buffer[position-11+20])<<16)+(long)((buffer[position-11+27])<<8)+(long)(buffer[position-11+26]);
											//files++;
										}
										else
										{
											if(position==512)
											{
												//position=0;
											}
											else
											{
												position++;
											}
										}
									}
								}
							}
						}
						clean_name();
						if(file_dir[cuenta].name.file_dir_name[0]!=0xFF && file_dir[cuenta].name.file_dir_name[0]!=0x00)
						{
							if(file_dir[cuenta].type==IS_DIR)
							{
								//Leandro (28/02/2019): Ajuste para compatibilidade
								SD_IntToString(number, valor);
								prints(valor); prints(". (DIR)"); printTab(); printTab();
							}
							else
							{
								//Leandro (28/02/2019): Ajuste para compatibilidade
								SD_IntToString(number, valor);
								prints(valor); prints(". (FILE)"); printTab(); printTab();
							}
							char i;
							for(i=0;i<255;i++)
							{
								if(file_dir[cuenta].name.file_dir_name[i]!=0x00)
								{
									//Leandro (28/02/2019): Ajuste para compatibilidade
									printc(file_dir[cuenta].name.file_dir_name[i]);
								}
								else break;	//Teste
							}
							//printf("\t\t");
							//printf("%d/%d/%d	%d:%d\n",file_dir[cuenta].name.info.day,file_dir[cuenta].name.info.month,file_dir[cuenta].name.info.year,file_dir[cuenta].name.info.hour,file_dir[cuenta].name.info.minute);
							//Leandro (28/02/2019): Ajuste para compatibilidade
							printTab(); printTab();
							SD_IntToString(file_dir[cuenta].name.info.day, valor);		prints(valor); printc('/');
							SD_IntToString(file_dir[cuenta].name.info.month, valor);	prints(valor); printc('/');
							SD_IntToString(file_dir[cuenta].name.info.year, valor);		prints(valor); prints("  ");
							SD_IntToString(file_dir[cuenta].name.info.hour, valor);		prints(valor); printc(':');
							SD_IntToString(file_dir[cuenta].name.info.minute, valor);	prints(valor);
//Depura
							prints("  1oC ");
							SD_IntToString(file_dir[cuenta].name.info.first_cluster, valor); prints(valor);	
//EndDepura
							printEnter();
							
							cuenta++;
							number++;
						}
					}
					else
					{
						if(position==512)
						{
							//position=0;
						}
						else
						{
							position++;
						}
					}
				}
				else
				{
					if(position==512)
					{
						count--;
					}
					else
					{
						position++;
					}
				}
			} while (position<512);
			position=0;
			sectors_to_be_read--;
		}while(sectors_to_be_read>0);
	}
	send_command(CMD12,0,SSI_number);
	sectors_to_be_read=(next_cluster*4)/512;
	if(send_command(CMD18,fat_begin_lba,SSI_number)==0)
	{
		do
		{
			sector++;
			rcvr_datablock(buffer, 512,SSI_number);
		}while(sector<=sectors_to_be_read);
		sector--;
	}
	send_command(CMD12,0,SSI_number);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);
	if((next_cluster==0x0FFFFFFF || next_cluster==0xFFFFFFFF) && current_count<40 && subdirs==GET_SUBDIRS)
	{
		while(current_count<40&&file_dir[current_count].type!=IS_DIR)
		{
			current_count++;
		}
		if(current_count<40 && file_dir[current_count].type==IS_DIR)
		{
			//Leandro (28/02/2019): Ajuste para compatibilidade
			prints("Content of ");
			char i;
			for(i=0;i<255;i++)
			{
				if(file_dir[current_count].name.file_dir_name[i]!=0x00)
				{
					//Leandro (28/02/2019): Ajuste para compatibilidade
					printc(file_dir[current_count].name.file_dir_name[i]);					
				}
			}
			next_cluster=file_dir[current_count].name.info.first_cluster;
			current_count++;
			//Leandro (28/02/2019): Ajuste para compatibilidade
			printEnter(); //printTab();	
		}
	}
	if(current_count==40)
	{
		number=0;
		next_cluster=0x0FFFFFFF;
	}
	return next_cluster;
}

/*
 *Reads file content.
 *Please note that this method should be modified if the file to be opened is not a txt file (concretely the content inside the for loop)
 */
long SD_open_file_TXT(long next_cluster,enum SSI SSI_number)
{

	unsigned char buffer[512];
	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	if(send_command(CMD18,address,SSI_number)==0)
	{
		do
		{
			rcvr_datablock(buffer, 512,SSI_number);
			int c=0;
			for(c=0;c<512;c++)
			{
				//if(buffer[c]!=0x00)
				if((buffer[c]!=0x00) || (c<400))
				{
					//Leandro (28/02/2019): Ajuste para compatibilidade
					printc(buffer[c]);
				}
				else
				{
					c=512;
					finish=1;
				}
			}
			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1);
	}
	send_command(CMD12,0,SSI_number);
	sectors_to_be_read=(next_cluster*4)/512;
	if(send_command(CMD18,fat_begin_lba,SSI_number)==0)
	{
		do
		{
			sector++;
			rcvr_datablock(buffer, 512,SSI_number);
		}while(sector<=sectors_to_be_read);
		sector--;
	}
	send_command(CMD12,0,SSI_number);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);
	if(next_cluster==0x0FFFFFFF || next_cluster==0x0FFFFFFF)
	{
		finish=0;
	}
	return next_cluster;
}

/*
 * Removes non ascii characters
 * Note: It must be checked if it removes characters, for example, with accents
 */
void clean_name()
{
	char j,k;
	for(j=0;j<255;j++)
	{
		if(file_dir[cuenta].name.file_dir_name[j]<0x20 || file_dir[cuenta].name.file_dir_name[j]>0x7E)
		{
			k=j;
			do
			{
				k++;
			}while(k<255 && (file_dir[cuenta].name.file_dir_name[k]<0x20 || file_dir[cuenta].name.file_dir_name[k]>0x7E));
			file_dir[cuenta].name.file_dir_name[j]=file_dir[cuenta].name.file_dir_name[k];
			file_dir[cuenta].name.file_dir_name[k]=0x00;
		}
	}
}

/*
 * Initialises Timer 5
 */
void SD_Timer5_Init(void)
{

	volatile unsigned short delay;
	SYSCTL_RCGCTIMER_R |= 0x20;
	delay = SYSCTL_SCGCTIMER_R;
	delay = SYSCTL_SCGCTIMER_R;
	TIMER5_CTL_R = 0x00000000;       // 1) disable timer5A during setup
	TIMER5_CFG_R = 0x00000000;       // 2) configure for 32-bit mode
	TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
	TIMER5_TAILR_R = 159999;         // 4) reload value, 10 ms, 16 MHz clock
	TIMER5_TAPR_R = 0;               // 5) bus clock resolution
	TIMER5_ICR_R |= 0x00000001;       // 6) clear timer5A timeout flag
	TIMER5_IMR_R |= 0x00000001;       // 7) arm timeout interrupt
	NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000040; // 8) priority 2
	// interrupts enabled in the main program after all devices initialized
	// vector number 108, interrupt number 92
	NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
	TIMER5_CTL_R = 0x00000001;       // 10) enable timer5A
}

// Executed every 10 ms
//void Timer5A_Handler(void){
//Leandro (17/02/2019) - ajuntes para o padrão do CMSIS
void TIMER5A_Handler(void){
  TIMER5_ICR_R = 0x00000001;       // acknowledge timer5A timeout
  disk_timerproc();
}

void disk_timerproc(void){   /* Decrements timer */
  if(Timer1) Timer1--;
  if(Timer2) Timer2--;
}

/*
 * Receives a block from a read of an SD card
 */
unsigned int rcvr_datablock (
    unsigned char *buff,         /* Data buffer to store received data */
    unsigned int btr, enum SSI SSI_number){          /* Byte count (must be even number) */
	unsigned char token;
  Timer1 = 10;
  do {                            /* Wait for data packet in timeout of 100ms */
    token = sd_read(SSI_number);
  } while ((token == 0xFF) && Timer1);
  if(token != 0xFE) return 0;    /* If not valid data token, retutn with error */

  do {                            /* Receive the data block into buffer */
    rcvr_spi_m(buff++,SSI_number);
    rcvr_spi_m(buff++,SSI_number);
  } while (btr -= 2);
  sd_write(0xFF,SSI_number);                        /* Discard CRC */
  sd_write(0xFF,SSI_number);

  return 1;                    /* Return with success */
}

void rcvr_spi_m(unsigned char *dst,enum SSI SSI_number){
  *dst = sd_read(SSI_number);
}

//Leandro (26/02/2019): Função criada para depuração
//Mostra todo conteúdo do Master Boot Record (1o setor do SDCard não particionado)
void show_MBR(unsigned char boot[])
{	
/*
FAT32 é little endian (lendo da esquerda para direita, você encontra o LSB primeiro)	

Master Boot Record
Offset	Description													Size
000h		Executable Code (Boots Computer)		446 Bytes
1BEh		1st Partition Entry (See NextTable)	16 Bytes
1CEh		2nd Partition Entry									16 Bytes
1DEh		3rd Partition Entry									16 Bytes
1EEh		4th Partition Entry									16 Bytes
1FEh		Boot Record Signature (55hAAh)			2 Bytes
	
Offset	Description																															Size
00h			Current State of Partition(00h=Inactive, 80h=Active)										1 Byte
01h			Beginning of Partition - Head																						1 Byte
02h			Beginning of Partition - Cylinder/Sector (See Below)										1 Word
04h			Type of Partition (See List Below)																			1 Byte
05h			End of Partition - Head																									1 Byte
06h			End of Partition - Cylinder/Sector																			1 Word
08h			Number of Sectors Betweenthe MBR and the First Sector in the Partition	1 Double Word
0Ch			Number of Sectors in the Partition																			1 Double Word

FAT32 Boot Record Information
This information is located in the first sector of every partition.
Offset	Description																															Size
00h			Jump Code + NOP																													3 Bytes
03h			OEM Name (Probably MSWIN4.1)																						8 Bytes
0Bh			Bytes Per Sector																												1 Word
0Dh			Sectors Per Cluster																											1 Byte
0Eh			Reserved Sectors																												1 Word
10h			Number of Copies of FAT																									1 Byte
11h			Maximum Root DirectoryEntries (N/A for FAT32)														1 Word
13h			Number of Sectors inPartition Smaller than 32MB (N/A for FAT32)					1 Word
15h			Media Descriptor (F8h forHard Disks)																		1 Byte
16h			Sectors Per FAT in Older FAT Systems (N/A for FAT32)										1 Word
18h			Sectors Per Track																												1 Word
1Ah			Number of Heads																													1 Word
1Ch			Number of Hidden Sectors inPartition																		1 Double Word
20h			Number of Sectors in Partition																					1 Double Word
24h			Number of Sectors Per FAT																								1 Double Word
28h			Flags (Bits 0-4 IndicateActive FAT Copy) 
				(Bit 7 Indicates whether FAT Mirroringis Enabled or Disabled ) 
				(If FAT Mirroring is Disabled, the FAT Information is only written 
				to the copy indicated by bits 0-4)																			1 Word
2Ah			Version of FAT32 Drive 
				(HighByte = Major Version, Low Byte = Minor Version)										1 Word
2Ch			Cluster Number of the Startof the Root Directory												1 Double Word
30h			Sector Number of the FileSystem Information Sector 
				(See Structure Below)(Referenced from the Start of the Partition)				1 Word
32h			Sector Number of the Backup Boot Sector 
				(Referenced from the Start of the Partition)														1 Word
34h			Reserved																																12 Bytes
40h			Logical Drive Number of Partition	1 Byte
41h			Unused (Could be High Byteof Previous Entry)														1 Byte
42h			Extended Signature (29h)																								1 Byte
43h			Serial Number of Partition																							1 Double Word
47h			Volume Name of Partition																								11 Bytes
52h			FAT Name (FAT32)																												8 Bytes
5Ah			Executable Code																													420 Bytes
1FEh		Boot Record Signature (55hAAh)																					2 Bytes
*/
	unsigned char valor[11];
	unsigned int i;
	
	prints("Master Boot Record - 1o setor do SDCard"); printEnter();
	printEnter();
	prints("1BEh-1st Partition Entry - 16 Bytes"); printEnter();

	prints("00h-Current State of Partition(00h=Inactive, 80h=Active) = 0x");
	SD_IntToStringHexa((unsigned int)boot[0x1BE], valor);
	prints(valor); printEnter();
	
	prints("01h-Beginning of Partition - Head = ");
	SD_IntToString((unsigned int)boot[0x1BF], valor);
	prints(valor); printEnter();

	prints("02h-Beginning of Partition - Cylinder/Sector (See Below) = ");
	SD_IntToString(boot[0x1C0] + (unsigned int)(boot[0x1C1]<<8), valor);
	prints(valor); printEnter();
	
	prints("04h-Type of Partition (See List Below) = ");
	SD_IntToString((unsigned int)boot[0x1C2], valor);
	prints(valor); 
	switch(boot[0x1C2])
	{
		case 0x00: prints(" - Unknown or Nothing"); break;
		case 0x01: prints(" - 12-bit FAT"); break;
		case 0x04: prints(" - 16-bit FAT (Partition Smallerthan 32MB)"); break;
		case 0x05: prints(" - Extended MS-DOS Partition"); break;
		case 0x06: prints(" - 16-bit FAT (Partition Largerthan 32MB)"); break;
		case 0x0B: prints(" - 32-bit FAT (Partition Up to 2048GB)"); break;
		case 0x0C: prints(" - 32-bit FAT, but uses LBA1 13h Extensions"); break;
		case 0x0E: prints(" - 16-bit FAT, but uses LBA1 13h Extensions"); break;
		case 0x0F: prints(" - Extended MS-DOS Partitio, but uses LBA1 13h Extensions"); break;
	}	
	printEnter();

	prints("05h-End of Partition - Head = ");
	SD_IntToString((unsigned int)boot[0x1C3], valor);
	prints(valor); printEnter();
	
	prints("06h-End of Partition - Cylinder/Sector = ");
	SD_IntToString(boot[0x1C4] + (unsigned int)(boot[0x1C5]<<8), valor);
	prints(valor); printEnter();
	
	prints("08h-Number of Sectors Betweenthe MBR and the First Sector in the Partition = ");
	SD_IntToString(boot[0x1C6]+ (unsigned int)(boot[0x1C7]<<8) +
		(unsigned int)(boot[0x1C8]<<16) + (unsigned int)(boot[0x1C9]<<24), valor);
	prints(valor); printEnter();
	
	prints("0Ch-Number of Sectors in the Partition = ");
	SD_IntToString(boot[0x1CA] + (unsigned int)(boot[0x1CB]<<8) +
		(unsigned int)(boot[0x1CC]<<16) + (unsigned int)(boot[0x1CD]<<24), valor);
	prints(valor); printEnter();
	
	printEnter();
	
	prints("1FEh-Boot Record Signature (55hAAh)-2 Bytes: 0x");
	SD_IntToStringHexa((unsigned int)(boot[0x1FE]<<8) + boot[0x1FF], valor);
	prints(valor); printEnter();
	
	printEnter();
	
	prints("FAT32 Boot Record Information"); printEnter();
	prints("This information is located in the first sector of every partition"); printEnter();

	prints("00h-Jump Code + NOP: ");
	SD_IntToStringHexa((unsigned int)(boot[0x00]<<8 + boot[0x01]), valor);
	prints(valor); printc(' ');
	SD_IntToStringHexa((unsigned int)boot[0x02], valor);
	prints(valor); printEnter();
		
	prints("03h-OEM Name (Probably MSWIN4.1): ");
	for(i = 0x03; i < (0x03+8); i++)
		printc(boot[i]);
	printEnter();
	
	prints("0Bh-Bytes Per Sector: ");
	SD_IntToString(boot[0x0B] + (unsigned int)(boot[0x0C]<<8), valor);
	prints(valor); printEnter();
		
	prints("0Dh-Sectors Per Cluster: ");
	SD_IntToString((unsigned int)boot[0x0D], valor);
	prints(valor); printEnter();
	
	prints("0Eh-Reserved Sectors: ");
	SD_IntToString(boot[0x0E] + (unsigned int)(boot[0x0F]<<8), valor);
	prints(valor); printEnter();
	
	prints("10h-Number of Copies of FAT: ");
	SD_IntToString((unsigned int)boot[0x10], valor);
	prints(valor); printEnter();
	
	prints("11h-Maximum Root Directory Entries (N/A for FAT32): ");
	SD_IntToString(boot[0x11] + (unsigned int)(boot[0x12]<<8), valor);
	prints(valor); printEnter();
	
	prints("13h-Number of Sectors in Partition Smaller than 32MB (N/A for FAT32): ");
	SD_IntToString(boot[0x13] + (unsigned int)(boot[0x14]<<8), valor);
	prints(valor); printEnter();

	prints("15h-Media Descriptor (F8h forHard Disks): ");
	SD_IntToString((unsigned int)boot[0x15], valor);
	prints(valor); printEnter();
	
	prints("16h-Sectors Per FAT in Older FAT Systems (N/A for FAT32): ");
	SD_IntToString(boot[0x16] + (unsigned int)(boot[0x17]<<8), valor);
	prints(valor); printEnter();
	
	prints("18h-Sectors Per Track: ");
	SD_IntToString(boot[0x18] + (unsigned int)(boot[0x19]<<8), valor);
	prints(valor); printEnter();
	
	prints("1Ah-Number of Heads: ");
	SD_IntToString(boot[0x1A] + (unsigned int)(boot[0x1B]<<8), valor);
	prints(valor); printEnter();
	
	prints("1Ch-Number of Hidden Sectors in Partition: ");
	SD_IntToString(boot[0x1C]+ (unsigned int)(boot[0x1D]<<8) +
		(unsigned int)(boot[0x1E]<<16) + (unsigned int)(boot[0x1F]<<24), valor);
	prints(valor); printEnter();
	
	prints("20h-Number of Sectors in Partition: ");
	SD_IntToString(boot[0x20]+ (unsigned int)(boot[0x21]<<8) +
		(unsigned int)(boot[0x22]<<16) + (unsigned int)(boot[0x23]<<24), valor);
	prints(valor); printEnter();
	
	prints("24h-Number of Sectors Per FAT: ");
	SD_IntToString(boot[0x24]+ (unsigned int)(boot[0x25]<<8) +
		(unsigned int)(boot[0x26]<<16) + (unsigned int)(boot[0x27]<<24), valor);
	prints(valor); printEnter();

	prints("28h-Flags (Bits 0-4 IndicateActive FAT Copy)"); printEnter();
	prints("(Bit 7 Indicates whether FAT Mirroring is Enabled or Disabled )"); printEnter();
	prints("(If FAT Mirroring is Disabled, the FAT Information is only written"); printEnter();
	prints("to the copy indicated by bits 0-4): 0x");
	SD_IntToStringHexa(boot[0x28] + (unsigned int)(boot[0x29]<<8), valor);
	prints(valor); printEnter();
	
	prints("2Ah-Version of FAT32 Drive");	printEnter();
	prints("(HighByte = Major Version, Low Byte = Minor Version): ");
	SD_IntToStringHexa(boot[0x2A] + (unsigned int)(boot[0x2B]<<8), valor);
	prints(valor); printEnter();
		
	prints("2Ch-Cluster Number of the Start of the Root Directory: ");
	SD_IntToString(boot[0x2C]+ (unsigned int)(boot[0x2D]<<8) +
		(unsigned int)(boot[0x2E]<<16) + (unsigned int)(boot[0x2F]<<24), valor);
	prints(valor); printEnter();

	prints("30h-Sector Number of the File System Information Sector"); printEnter();
	prints("(See Structure Below)(Referenced from the Start of the Partition): ");
	SD_IntToString(boot[0x30] + (unsigned int)(boot[0x31]<<8), valor);
	prints(valor); printEnter();
		
	prints("32h-Sector Number of the Backup Boot Sector"); 
	prints("(Referenced from the Start of the Partition): ");
	SD_IntToString(boot[0x32] + (unsigned int)(boot[0x33]<<8), valor);
	prints(valor); printEnter();
		
	prints("34h-Reserved");
	printEnter();
	
	prints("40h-Logical Drive Number of Partition: ");
	SD_IntToString((unsigned int)boot[0x40], valor);
	prints(valor); printEnter();
	
	prints("41h-Unused (Could be High Byte of Previous Entry): 0x");
	SD_IntToStringHexa((unsigned int)boot[0x41], valor);
	prints(valor); printEnter();
	
	prints("42h-Extended Signature (29h): ");
	SD_IntToStringHexa((unsigned int)boot[0x42], valor);
	prints(valor); printEnter();
	
	prints("43h-Serial Number of Partition: ");
	SD_IntToString(boot[0x43]+ (unsigned int)(boot[0x44]<<8) +
		(unsigned int)(boot[0x45]<<16) + (unsigned int)(boot[0x46]<<24), valor);
	prints(valor); printEnter();

	prints("47h-Volume Name of Partition: ");
	for(i = 0x47; i < (0x47+11); i++)
		printc(boot[i]);
	printEnter();
	
	prints("52h-FAT Name (FAT32): ");
	for(i = 0x52; i < (0x52+8); i++)
		printc(boot[i]);
	printEnter();
	
	prints("5Ah-Executable Code (420 Bytes): ");
	printEnter();
	for(i = 0x5A; i < (0x5A+420); i++)
		printc(boot[i]);
	printEnter();
	
	prints("1FEh-Boot Record Signature (55hAAh): 0x");
	SD_IntToStringHexa((unsigned int)(boot[0x1FE]<<8) + boot[0x1FF], valor);
	prints(valor); printEnter();
}

////Define um ponteiro para funções do tipo TipoFuncao
//TipoFuncaoString *prints;
//TipoFuncaoChar *printc;
//Função para definição de função de impressão de string
void SD_define_print_function(TipoFuncaoString *PonteiroParaUmaFuncaoString, TipoFuncaoChar *PonteiroParaUmaFuncaoChar)
{
	prints = PonteiroParaUmaFuncaoString;
	printc = PonteiroParaUmaFuncaoChar;
#ifdef depura
	prints("Teste 123"); printc(0x0D); printc(0x0A);
#endif
}

//Conversão de inteiro para string decimal
static void SD_IntToString(unsigned int n, unsigned char* s)
{
	unsigned int i,aux1;
	char first = 1;
	for(i = 1000000000; i >= 10; i/=10)
	{
		aux1 = n/i;
		n = n%i;
		if((aux1 > 0) || (first == 0))
		{
			first = 0;
			*s = aux1 + '0';
			s++;
		}		
	}
	*s = n + '0';	//Unidade
	s++;
	*s = 0;	//Terminador nulo	
}

//Conversão de inteiro para string hexa
static void SD_IntToStringHexa(unsigned int n, unsigned char* s)
{
	int i;
	unsigned int aux1;
	for(i = 7; i >= 0; i--)
	{
		aux1 = n & 0x0000000F;
		if(aux1 <= 9)
			*(s+i) = aux1 + '0';
		else
			*(s+i) = aux1 + 55;
		n >>= 4;
	}
	*(s+8) = 0;	//Terminador nulo	
}

//Conversão de string para inteiro
static unsigned int SD_StringToInt(unsigned char* s)
{
	unsigned int i=0,num=0;
	unsigned char temp;
	temp = *s;
	while((temp!=0) && (i<=10))
	{
		num *= 10;
		num += temp - '0';
		i++;
		temp = *(s+i);
	}
	return num;
}

//Abre um arquivo do tipo BMP
long SD_open_file_BMP(long next_cluster, int first, enum SSI SSI_number)
{
	static unsigned int tamanho;
	unsigned char buffer[512], aux;
	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	//READ_MULTIPLE_BLOCK
	if(send_command(CMD18,address,SSI_number)==0)
	{
		do
		{
			rcvr_datablock(buffer, 512, SSI_number);
			//Testa para saber se é o primeiro setor lido (cabeçalho)
			if(first)
			{
				first = 0;
				//Lê o valor do tamanho do arquivo em bytes
				tamanho = buffer[0x02] + (unsigned int)(buffer[0x03]<<8) + (unsigned int)(buffer[0x04]<<16) + (unsigned int)(buffer[0x05]<<24);
				//Mostra todo cabeçalho
				show_BMP_Header(buffer);
//break;				
			}
			int c=0;
			for(c=0;c<512;c++)
			{
				if(tamanho--)
				{
					aux = (buffer[c]>>4) & 0x0000000F;
					if(aux <= 9)	aux = aux + '0';
					else					aux = aux + 55;					
					printc(aux);
					aux = buffer[c] & 0x0000000F;
					if(aux <= 9)	aux = aux + '0';
					else					aux = aux + 55;					
					printc(aux);
					printc(' ');
				}
				else
				{
					c=512;
					finish=1;
				}
			}
			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1);
	}
	//STOP_TRANSMISSION
	send_command(CMD12,0,SSI_number);
	sectors_to_be_read=(next_cluster*4)/512;
	//READ_MULTIPLE_BLOCK
	if(send_command(CMD18,fat_begin_lba,SSI_number)==0)
	{
		do
		{
			sector++;
			rcvr_datablock(buffer, 512,SSI_number);
		}while(sector<=sectors_to_be_read);
		sector--;
	}
	//STOP_TRANSMISSION
	send_command(CMD12,0,SSI_number);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);
	if(next_cluster==0x0FFFFFFF || next_cluster==0x0FFFFFFF)
	{
		finish=0;
	}
	return next_cluster;
}

//Leandro (28/02/2019): Função criada para depuração
//Mostra todo conteúdo do cabeçalho de um arquivo BMP
void show_BMP_Header(unsigned char header[])
{
/*
ESTRUTURA DETALHADA DO FORMATO BMP
A) Cabeçalho de arquivo – informações do arquivo - Tamanho : 14 bytes
Campo 				Bytes Descrição
BfType 				2 		Assinatura do arquivo: os caracteres ASCII "BM" ou (424D)h. É a identificação de ser realmente BMP.
BfSize 				4 		Tamanho do arquivo em Bytes
BfReser1 			2 		Campo reservado 1; deve ser ZERO
BfReser2 			2 		Campo reservado 2; deve ser ZERO
BfOffSetBits 	4 		Especifica o deslocamento, em bytes, para o início da área de dados da imagem, a partir do início deste cabeçalho.
										- Se a imagem usa paleta, este campo tem tamanho=14+40+(4 x NumeroDeCores)
										- Se a imagem for true color, este campo tem tamanho=14+40=54

B) Cabeçalho de mapa de bits – informações da imagem - Tamanho : 40 bytes
Campo 				Bytes Descrição
BiSize 				4 		Tamanho deste cabeçalho (40 bytes). Sempre (28)h.
BiWidth 			4 		Largura da imagem em pixels
BiHeight 			4 		Altura da imagem em pixels
BiPlanes 			2 		Número de planos de imagem. Sempre 1
BiBitCount 		2 		Quantidade de Bits por pixel (1,4,8,24,32)
										Este campo indica, indiretamente, ainda o número máximo de cores, que é 2^(Bits por pixel)
BiCompress 		4 		Compressão usada. Pode ser:
										0 = BI_RGB _ sem compressão
										1 = BI_RLE8 – compressão RLE 8 bits
										2 = BI_RLE4 – compressão RLE 4 bits
BiSizeImag 		4 		Tamanho da imagem (dados) em byte
										- Se arquivo sem compressão, este campo pode ser ZERO.
										- Se imagem em true color, será Tamanho do arquivo (Bfsize) menos deslocamento (BfOffSetBits)
BiXPPMeter 		4 		Resolução horizontal em pixels por metro
BiYPPMeter 		4 		Resolução vertical em pixels por metro
BiClrUsed 		4 		Número de cores usadas na imagem. Quando ZERO indica o uso do máximo de cores possível 
										pela quantidade de Bits por pixel , que é 2^(Bits por pixel)
BiClrImpor 		4 		Número de cores importantes (realmente usadas) na imagem.
										Por exemplo das 256 cores, apenas 200 são efetivamente usadas.
										Se todas são importantes pode ser ZERO. É útil quando for
										exibir uma imagem em 1 dispositivo que suporte menos cores
										que a imagem possui.

C) Paleta cores - definição tabela de cores - Tamanho : 4 bytes x Número de Cores
Campo 				Bytes Descrição
Blue 					1 		Intensidade de Azul. De 0 a 255
Green 				1 		Intensidade de Verde. De 0 a 255
Red 					1 		Intensidade de Vermelho. De 0 a 255
Reservado 		1 		Campo reservado deve ser ZERO sempre

D) Área de dados da imagem - cor que cada pixel deve ser ligado ou esses dados
comprimidos - Tamanho: campo BiSizeImg, do cabeçalho de informações da
imagem.
O primeiro pixel refere-se a posição inferior esquerda.
O último pixel refere-se a posição superior direita.
O valor lido nessa área de dados, se sem compressão, refere-se a cor do pixel de acordo
com a tabela de cores (paleta).
Há, entretanto, a restrição de que cada linha deva ter N bytes, sendo N um número
divisível por 4. Caso contrário, o BMP deve ser preenchido com bytes não válidos. Por
exemplo, se a imagem tem 1 x 100 pixels em 8 bits/pixel, o BMP teria 1 byte válido em
cada linha e mais 3 bytes que não tem qualquer significado.
No BMP true color (24 bits) cada sequência de 3 bytes correspondem a uma sequência
Blue.Green.Red , isso é a composição da cor do pixel diretamente (não tendo neste caso
paleta de cores). No true color de 32 bits, é a combinação de 4 bytes que estabelecerá a
cor do pixel.
*/
	unsigned char valor[11];
	unsigned int i;
	
	prints("Bytes Hexa no formato little endian"); printEnter(); printEnter();
	prints("A) Cabecalho de arquivo - informacoes do arquivo - Tamanho : 14 bytes"); printEnter();
	prints("Campo         Bytes Valor (H) Valor (10)"); printEnter();
	prints("BfType        2     "); SD_IntToStringHexa(header[0x01] + (unsigned int)(header[0x00]<<8), valor); prints(valor);	prints("  ASCII 'BM'"); printEnter();
	prints("BfSize        4     "); SD_IntToStringHexa(header[0x05] + (unsigned int)(header[0x04]<<8) + (unsigned int)(header[0x03]<<16) + (unsigned int)(header[0x02]<<24), valor); prints(valor); prints("  "); SD_IntToString(header[0x02] + (unsigned int)(header[0x03]<<8) + (unsigned int)(header[0x04]<<16) + (unsigned int)(header[0x05]<<24), valor); prints(valor); printEnter();
	prints("BfReser1      2     "); SD_IntToStringHexa(header[0x07] + (unsigned int)(header[0x06]<<8), valor); prints(valor); prints("  "); SD_IntToString(header[0x06] + (unsigned int)(header[0x07]<<8), valor); prints(valor); printEnter();
	prints("BfReser2      2     "); SD_IntToStringHexa(header[0x09] + (unsigned int)(header[0x08]<<8), valor); prints(valor); prints("  ");	SD_IntToString(header[0x08] + (unsigned int)(header[0x09]<<8), valor); prints(valor); printEnter();
	prints("BfOffSetBits  4     "); SD_IntToStringHexa(header[0x0D] + (unsigned int)(header[0x0C]<<8) + (unsigned int)(header[0x0B]<<16) + (unsigned int)(header[0x0A]<<24), valor); prints(valor); prints("  "); SD_IntToString(header[0x0A] + (unsigned int)(header[0x0B]<<8) + (unsigned int)(header[0x0C]<<16) + (unsigned int)(header[0x0D]<<24), valor); prints(valor); printEnter();
	printEnter();
	prints("B) Cabecalho de mapa de bits - informacoes da imagem - Tamanho : 40 bytes"); printEnter();
	prints("Campo         Bytes Valor (H) Valor (10)"); printEnter();
	prints("BiSize        4     "); SD_IntToStringHexa(header[0x11] + (unsigned int)(header[0x10]<<8) + (unsigned int)(header[0x0F]<<16) + (unsigned int)(header[0x0E]<<24), valor); prints(valor); prints("  ");	SD_IntToString(header[0x0E] + (unsigned int)(header[0x0F]<<8) + (unsigned int)(header[0x10]<<16) + (unsigned int)(header[0x11]<<24), valor); prints(valor); printEnter();
	prints("BiWidth       4     "); SD_IntToStringHexa(header[0x15] + (unsigned int)(header[0x14]<<8) + (unsigned int)(header[0x13]<<16) + (unsigned int)(header[0x12]<<24), valor); prints(valor); prints("  ");	SD_IntToString(header[0x12] + (unsigned int)(header[0x13]<<8) + (unsigned int)(header[0x14]<<16) + (unsigned int)(header[0x15]<<24), valor); prints(valor); printEnter();
	prints("BiHeight      4     "); SD_IntToStringHexa(header[0x19] + (unsigned int)(header[0x18]<<8) + (unsigned int)(header[0x0F]<<17) + (unsigned int)(header[0x16]<<24), valor); prints(valor); prints("  ");	SD_IntToString(header[0x16] + (unsigned int)(header[0x17]<<8) + (unsigned int)(header[0x18]<<16) + (unsigned int)(header[0x19]<<24), valor); prints(valor); printEnter();
	prints("BiPlanes      2     "); printEnter();
	prints("BiBitCount    2     "); SD_IntToStringHexa(header[0x1D] + (unsigned int)(header[0x1C]<<8), valor); prints(valor); prints("  ");	SD_IntToString(header[0x1C] + (unsigned int)(header[0x1D]<<8), valor); prints(valor); printEnter();
	prints("BiCompress    4     "); printEnter();
	prints("BiSizeImag    4     "); SD_IntToStringHexa(header[0x25] + (unsigned int)(header[0x24]<<8) + (unsigned int)(header[0x23]<<16) + (unsigned int)(header[0x22]<<24), valor); prints(valor); prints("  "); SD_IntToString(header[0x22] + (unsigned int)(header[0x23]<<8) + (unsigned int)(header[0x24]<<16) + (unsigned int)(header[0x25]<<24), valor); prints(valor); printEnter();
	prints("BiXPPMeter    4     "); printEnter();
	prints("BiYPPMeter    4     "); printEnter();
	prints("BiClrUsed     4     "); printEnter();
	prints("BiClrImpor    4     "); printEnter();
	printEnter();
}

//Leandro (03/03/2019): Abre um arquivo do tipo BMP crú e envia pela função de transferência de strings
long SD_open_file_BMP_raw(long next_cluster, int first, enum SSI SSI_number)
{
	static unsigned int tamanho;
	unsigned char buffer[512], aux;
	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	
	//Habilita o SD Card **************************
	SD_cs_low(SSI_number);
	
	//READ_MULTIPLE_BLOCK
	if(send_command(CMD18,address,SSI_number)==0)
	{
		do
		{
			rcvr_datablock(buffer, 512, SSI_number);
			//Testa para saber se é o primeiro setor lido (cabeçalho)
			if(first)
			{
				first = 0;
				//Lê o valor do tamanho do arquivo em bytes
				tamanho = buffer[0x02] + (unsigned int)(buffer[0x03]<<8) + (unsigned int)(buffer[0x04]<<16) + (unsigned int)(buffer[0x05]<<24);
				//Mostra todo cabeçalho
//			show_BMP_Header(buffer);
//break;				
			}
			int c=0;
			for(c=0;c<512;c++)
			{
				if(tamanho--)
				{
//					aux = (buffer[c]>>4) & 0x0000000F;
//					if(aux <= 9)	aux = aux + '0';
//					else					aux = aux + 55;					
//					printc(aux);
//					aux = buffer[c] & 0x0000000F;
//					if(aux <= 9)	aux = aux + '0';
//					else					aux = aux + 55;					
//					printc(aux);
//					printc(' ');
				}
				else
				{
					c=512;
					finish=1;
				}
			}
			
			prints(buffer);
			
			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1);
	}
	//STOP_TRANSMISSION
	send_command(CMD12,0,SSI_number);
	sectors_to_be_read=(next_cluster*4)/512;
	//READ_MULTIPLE_BLOCK
	if(send_command(CMD18,fat_begin_lba,SSI_number)==0)
	{
		do
		{
			sector++;
			rcvr_datablock(buffer, 512,SSI_number);
		}while(sector<=sectors_to_be_read);
		sector--;
	}
	//STOP_TRANSMISSION
	send_command(CMD12,0,SSI_number);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);
	if(next_cluster==0x0FFFFFFF || next_cluster==0x0FFFFFFF)
	{
		finish=0;
	}
	
	//Desabilita o SD Card **************************
	SD_cs_high(SSI_number);
	
	return next_cluster;
}

//Leandro (05/03/2019): Solicitar o primeiro cluster de um arquivo através de seu nome
long SD_get_first_cluster_name(unsigned char* name)
{
	unsigned int len, i, j, pos;
	
	len = 0;
	for(i=0;i<255;i++)
	{
		if(name[i] != NULL) len++;
		else						break;
	}
	
	//Carrega num com um valor maior que o vetor de arquivos
	pos = 40;
	//Procura em todos os arquivos
	for(i=0;i<40;i++)
	{
		//Procura em cada uma das letras
		for(j=0;j<255;j++)
		{
			//Se os caracteres são iguais
			if(name[j] == file_dir[i].name.file_dir_name[j])
			{
				//Se chegou no caractere nulo (NULL)
				if(j == len)
				{
					pos = i;	//Define o número do arquivo
					break;
				}
			}
			else
				break;	//Vai para o próximo arquivo
		}
		//Se achou o número, encerra a busca
		if(pos < 40)
				break;
	}
	//Se é um arquivo válido
	if(pos < 40)	
		return file_dir[pos].name.info.first_cluster;	//Retorna o número do 1o cluster
	else
		return 0;
}

//Leandro (05/03/2019): Solicitar o número do arquivo através de seu nome
long SD_get_number_name(unsigned char* name)
{
	unsigned int len, i, j, pos;
	
	len = 0;
	for(i=0;i<255;i++)
	{
		if(name[i] != NULL) len++;
		else						break;
	}
	
	//Carrega num com um valor maior que o vetor de arquivos
	pos = 40;
	//Procura em todos os arquivos
	for(i=0;i<40;i++)
	{
		//Procura em cada uma das letras
		for(j=0;j<255;j++)
		{
			//Se os caracteres são iguais
			if(name[j] == file_dir[i].name.file_dir_name[j])
			{
				//Se chegou no caractere nulo (NULL)
				if(j == len)
				{
					pos = i;	//Define o número do arquivo
					break;
				}
			}
			else
				break;	//Vai para o próximo arquivo
		}
		//Se achou o número, encerra a busca
		if(pos < 40)
				break;
	}
	//Se é um arquivo válido
	if(pos < 40)	
		return pos;	//Retorna o número do arquivo
	else
		return 40;
}

