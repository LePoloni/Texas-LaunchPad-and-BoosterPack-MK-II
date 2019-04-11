//*****************************************************************************
//	Includes
//*****************************************************************************
//CMSIS
#include "TM4C123GH6PM.h"						//CMSIS Cortex-M4 Peripheral Access Layer

//ARMCC (ARM C Compiler)
#include <stdint.h>									//Definição de tipos inteiros, máximos e mínimos
#include <stdbool.h>								//Definição de tipo booleano
#include <math.h>
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

//Valvano
#include "tm4c123gh6pmX.h"
#include "BSP.h"										//Foi adaptada para esses projetos

//SD Card
#include "sdcard.h"
#include <stdio.h>

//LaunchPad (criação própria)
#include "LaunchPad.h"

//MK II (criação própria)
#include "MKII.h"

//Mario Tunes (adaptação de Arduino)
#include "MarioTunes.h"

//*****************************************************************************
//	Variáveis Globais
//*****************************************************************************
//GPIOF	4		3			2		 1	 0
//			SW1	Green	Blue Red SW2

uint32_t BitmapStart = 0;
extern unsigned int msTicks; 			//Quantidade de ms
extern const unsigned short mario_60x40[];
extern const unsigned short divertidance_64x64[];
extern const unsigned short palito1_24x46[];
extern const unsigned short palito2_24x46[];
//extern const unsigned short eu_2016[];
//extern const unsigned short eu_2016_inv[];
//extern const unsigned char eu_2016_128x128_inv_16_cores[];
//extern const unsigned short eu_2016_128x128_inv[]; //excede o limite de memória
extern const int theme_melody[];
extern const int theme_tempo[];
extern const int underworld_melody[];
extern const int underworld_tempo[];
bool lado = 0;

//*****************************************************************************
//	Protótipo de Funções
//*****************************************************************************
void LCD_TxString(unsigned char* data);
unsigned int TransfBitmatFromSDCardToLCD(unsigned int num_arquivo);
void play_tunes(const int tune[], const int tempo[], int tamanho, int repete);

//*****************************************************************************
//	Função Principal
//*****************************************************************************
//Função principal controla led pela UART
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	//Setup
	//PLL_Inicializa();
	GPIOF_Inicializa();
	//UART0_Inicializa_115200bps_80MHz();
	UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intrinsecamente)
	
	while(1)
	{
		//Lê um comando
		dado = UART0_RxChar();
		//Eco
		UART0_TxChar(dado);
		//Trata o comando
		if(dado == '1')
		{
			GPIOF->DATA = GPIOF->DATA | Red;
			UART0_TxChar('O');UART0_TxChar('n');
		}
		else if(dado == '0')
		{
			GPIOF->DATA = GPIOF->DATA & ~Red;
			UART0_TxChar('O');UART0_TxChar('f');UART0_TxChar('f');
		}
				
		//Pisca-pisca por intrrupção do SysTick (usar SysTick_Config(_))
		if(msTicks >= 1000)
		{
			GPIOF->DATA = GPIOF->DATA ^ Blue;
			msTicks = 0;
		}
		
		//Pisca-pisca por delay com SysTick (usar SysTick_Inicializa())
		SysTick_Wait10ms(100);
		GPIOF->DATA = GPIOF->DATA ^ Green;
	}
}
//*/
//Função principal LCD TFT (chip ST7735), servo e buzzer
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t temp, umi, i;

	//Setup
	//PLL_Inicializa();
	//GPIOF_Inicializa();
	LedRed_Inicializa();
	//Profile_Init();
	//UART0_Inicializa_115200bps_80MHz();
	UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intrinsecamente)
	
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0, 0));
	
	//Servo_Init();					//Biblioteca LaunchPad
	MKII_Servo_Init();		//Biblioteca MK_II
	//Servo_Set_Angle(0);
	
	while(1)
	{
		if(UARTCharsAvail(UART0_BASE))	//TivaWare - se tem um byte disponível...
		{
			//Lê um comando
			dado = UART0_RxChar();
			//Eco
			UART0_TxChar(dado);
			//Trata o comando
			switch(dado)
			{
				case 'a': BSP_LCD_FillScreen(LCD_YELLOW);
					break;
				case 'b':	BSP_LCD_FillScreen(LCD_GREEN);
					break;
				case 'c': BSP_LCD_FillRect(10, 20, 108, 30, LCD_BLUE);
					break;
				case 'd': BSP_LCD_DrawString(2, 5, "POLONI", LCD_WHITE);	//COLUNA, LINHA
					break;
				case 'e': BSP_LCD_DrawChar(10, 60, '?', LCD_WHITE, LCD_BLUE, 4);	//PIXEL X, PIXEL Y (LEFT, TOP)
					break;
				case 'f': BSP_LCD_SetCursor(4, 10); BSP_LCD_OutUDec5(2019, LCD_MAGENTA);
					break;
				case 'g': BSP_LCD_Drawaxes(LCD_YELLOW, LCD_BLACK, "Tempo", "Temp", LCD_GREEN, "Umi", LCD_CYAN, 1000, -1000);
					break;
				case 'h': temp=1000; umi=-1000;
									while(temp>-1000)
									{
										BSP_LCD_PlotPoint(temp, LCD_GREEN);
										BSP_LCD_PlotPoint(umi, LCD_CYAN);
										BSP_LCD_PlotIncrement();
										temp -= 10;
										umi +=10;
										SysTick_Wait10ms(20); 
									}
					break;
				case 'i': BSP_LCD_DrawBitmap(0, 127, mario_60x40, 60, 40);
									//As imagens podem ser gerados no site http://www.rinkydinkelectronics.com/t_imageconverter565.php
									//Elas devem estar de ponta cabeça para que sejam desenhadas corretamente
					break;
				case 'j': for(i = 0; i<30; i++)
									{
										BSP_LCD_DrawPixel(30+i, 30+2*i, LCD_WHITE);
									}
					break;
				case 'k': BSP_LCD_DrawFastVLine(64, 0, 128, LCD_WHITE);
									BSP_LCD_DrawFastVLine(65, 0, 128, LCD_WHITE);
									BSP_LCD_DrawFastVLine(66, 0, 128, LCD_WHITE);
					break;
				case 'l': BSP_LCD_DrawFastHLine(0, 64, 128, LCD_CYAN);
									BSP_LCD_DrawFastHLine(0, 65, 128, LCD_CYAN);
									BSP_LCD_DrawFastHLine(0, 66, 128, LCD_CYAN);
					break;	
				//case 'm': BSP_LCD_DrawBitmap4Bits(0, 127, eu_2016_128x128_inv_16_cores, 128, 128);
					//break;	
				case 'n': BSP_LCD_DrawString(2, 5, "-90 graus", LCD_WHITE);	//COLUNA, LINHA
									//Servo_Set_Angle(-90);
									MKII_Servo_Set_Angle(-90);									
					break;	
				case 'o': BSP_LCD_DrawString(2, 5, "+90 graus", LCD_WHITE);	//COLUNA, LINHA
									//Servo_Set_Angle(+90);
									MKII_Servo_Set_Angle(+90);
					break;
				case 'p': BSP_LCD_Rect(10, 20, 108, 30, LCD_BLUE);
					break;
				case 'q': //BSP_LCD_Circle(64, 64, 50, LCD_BLUE);
									//BSP_LCD_Circle(0, 127, 100, LCD_BLUE);
									for(i=5; i<64; i=i+5)
									{
										BSP_LCD_Circle(64, 64, i, LCD_BLUE);
									}
					break;
				case 'r': BSP_LCD_FillCircle(30, 30, 20, LCD_BLUE);
									BSP_LCD_FillCircle(90, 30, 20, LCD_GREEN);
									BSP_LCD_FillCircle(30, 90, 20, LCD_CYAN);
									BSP_LCD_FillCircle(90, 90, 20, LCD_RED);
					break;
				case 's': //MKII_Buzzer_Init(440, 50);
									MKII_Timer1_PWM_Init(440, 50);
									msTicks=0; while(msTicks<200);
									//MKII_Buzzer_Set(2048, 50);
									MKII_Timer1_PWM_Set(2048, 50);
									msTicks=0; while(msTicks<200);
									//MKII_Buzzer_Set(2048, 0);
									MKII_Timer1_PWM_Set(2048, 0);								
					break;								
			}			
		}
		
		
		//Pisca-pisca por intrrupção do SysTick (usar SysTick_Config(_))
		if(msTicks >= 1000)
		{
			GPIOF->DATA = GPIOF->DATA ^ Red;
			msTicks = 0;
		}		
	}
}
//*/
//Função principal lê sensor de temparatura interno e joystick
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t x, y, select;
	long adc;
	long temp_int;

	//Setup
	PLL_Inicializa();
	//GPIOF_Inicializa();
	Profile_Init();
	UART0_Inicializa_115200bps_80MHz();
	//UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	//SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intersecamente)
	SysTick_Config(80000000/ 1000);      // 1 int./ms (interrupção configurada intersecamente)
	
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	
	ADC0_InitSWTriggerSeq3_Ch1();
	BSP_Joystick_Init();
	
	EdgeCounter_Init();
	
	BSP_LCD_DrawString(0, 3, "Temp = ", LCD_YELLOW);
	BSP_LCD_DrawString(0, 5, "X =    ", LCD_YELLOW);
	BSP_LCD_DrawString(0, 7, "Y =    ", LCD_YELLOW);
	BSP_LCD_DrawString(0, 9, "Sel =  ", LCD_YELLOW);
	
	while(1)
	{
		//Lê o sensor de temperatura interno do uC
		adc = ADC0_InSeq3();
		temp_int = 1475 - ((75 * 33 * adc) / 4096);
		if(temp_int >= 0)
		{
			BSP_LCD_DrawString(7, 3, "+", LCD_YELLOW);
			BSP_LCD_SetCursor(8, 3);
			BSP_LCD_OutUFix2_1(temp_int, LCD_YELLOW);				
		}
		else
		{
			BSP_LCD_DrawString(7, 3, "-", LCD_YELLOW);
			BSP_LCD_SetCursor(8, 3);
			BSP_LCD_OutUFix2_1(-temp_int, LCD_YELLOW);
		}
		
		//Lê o joystick da MKII
		BSP_Joystick_Input(&x, &y, &select);
		
		BSP_LCD_SetCursor(7, 5);
		BSP_LCD_OutUDec4(x, LCD_YELLOW);
		BSP_LCD_SetCursor(7, 7);
		BSP_LCD_OutUDec4(y, LCD_YELLOW);
		BSP_LCD_SetCursor(7, 9);
		BSP_LCD_OutUDec4(select, LCD_YELLOW);
		if(select)
			BSP_LCD_DrawString(0, 10, "Released", LCD_RED);
		else
			BSP_LCD_DrawString(0, 10, "Pressed!", LCD_GREEN);
		BSP_LCD_SetCursor(0, 11);
		BSP_LCD_OutUDec4(FallingEdges, LCD_YELLOW);
		
		//Pisca-pisca por interrupção do SysTick (usar SysTick_Config(_))
		while(msTicks <= 200);
		GPIOF->DATA = GPIOF->DATA ^ Red;
		msTicks = 0;				
	}
}
//*/
//Função principal SD Card
/*
int main()
{
	//Variáveis locais
	unsigned char arquivo[10];
	unsigned int num_arquivo, i;
	//Setup
	GPIOF_Inicializa();
	GPIOF->DATA |= Red;

	UART0_Inicializa_115200bps_16MHz();
	UART0_TxString("========> Teste SD Card <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);

	//SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);     //1 int./ms (interrupção configurada intersecamente)
	//SD Card
	Timer5_Init();															//Timeout do SD Card
	startSSI2();
	
	initialize_sd(SSI_2);
	cs_high(SSI_2);
  change_speed(SSI_2);
	cs_low(SSI_2);
	
	UART0_TxString("========> Fim da Inicializacao <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	
	UART0_TxString("========> Leitura do 1o Setor <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	read_first_sector(SSI_2);
	
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> Leitura de Dados <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	read_disk_data(SSI_2);
	long next_cluster=get_root_dir_first_cluster();
	
	printf("========> List dir and files <========");	//Em Manage Run-Time -> Compiler -> STDOUT/IN/ERR -> marcar e selecionar ITM
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> List dirs and files  <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	do
	{
		next_cluster=list_dirs_and_files(next_cluster,LONG_NAME,GET_SUBDIRS,SSI_2);
	}
	while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
	
	printf("\nDirs and files listed\n\n");
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> Dirs and files listed <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	
	while(1)
	{
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("========> File to be read <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("Digite o numero do arquivo a ser lido: ");
		
		//Lê uma string até o enter ou qualquer caractere não numérico
		i=0;
		for(;;)
		{
			arquivo[i] = UART0_RxChar();
			//UART0_TxChar(arquivo[i]);
			if(arquivo[i] < '0') break;
			if(arquivo[i] > '9') break;
			i++;
			if(i>8) break;
		}	
		arquivo[i] = 0;	//Terminador
		//UART0_TxString("Recebido: "); UART0_TxString(arquivo); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		//Converte para inteiro
		num_arquivo = StringToInt(arquivo);
		//UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		//UART0_TxString("Confirmacao: "); IntToString(num_arquivo, arquivo); UART0_TxString(arquivo);
		//UART0_TxChar(0x0D); UART0_TxChar(0x0A);

		//Seleciona o arquivo (1o cluster)
		next_cluster=get_first_cluster(num_arquivo);
		//next_cluster=get_first_cluster(5);
		
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("Conteudo do arquivo lido: "); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		//Mostra o arquivo
		do
		{
			next_cluster=open_file(next_cluster,SSI_2);
		}
		while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
	}
	return 0;
}
//*/
//Função principal SD Card
/*
int main()
{
	//Variáveis locais
	unsigned char arquivo[255], tipo;
	unsigned int num_arquivo, i;
	//Setup
	//GPIOF_Inicializa();
	LedRed_Inicializa();	
	GPIOF->DATA |= Red;
	UART0_Inicializa_115200bps_16MHz();	
	BSP_LCD_Init();
  	
	BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));

	UART0_TxString("========> Teste SD Card <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);

	//SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);     //1 int./ms (interrupção configurada intersecamente)
	
	//SD Card -------------------------------------------------------------------
	//Define as funções para envio de mensagens do SD Card
	SD_define_print_function(UART0_TxString, UART0_TxChar);
	//Inicializa o Timer5
	SD_Timer5_Init();							//Timeout do SD Card
	//Iniciliza o SSI2
	SD_startSSI2();								//OK: RB5 - X Joystick / CS - SDCard --> trocar para RE0
	//Inicializa o SD Card
	while(SD_initialize_sd(SSI_2));	//Aguarda inicializar com sucesso
	//Desabilita o SD Card
	SD_cs_high(SSI_2);
	//Altera a velocidade de comunicação para 8 Mbps
  SD_change_speed(SSI_2);
	//Habilita o SD Card
	SD_cs_low(SSI_2);
	
	UART0_TxString("========> Fim da Inicializacao <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	
	UART0_TxString("========> Leitura do 1o Setor <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	SD_read_first_sector(SSI_2);
	
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> Leitura de Dados <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	SD_read_disk_data(SSI_2);
	long next_cluster=SD_get_root_dir_first_cluster();
	
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> List dirs and files  <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	do
	{
		next_cluster=SD_list_dirs_and_files(next_cluster,LONG_NAME,GET_SUBDIRS,SSI_2);
	}
	while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
	
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxString("========> Dirs and files listed <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	
	while(1)
	{
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("========> File to be read <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("Digite o numero do arquivo a ser lido: ");
		
		//Lê uma string até o enter ou qualquer caractere não numérico
		i=0;
		for(;;)
		{
			arquivo[i] = UART0_RxChar();
			//UART0_TxChar(arquivo[i]);		//Eco
			if(arquivo[i] < '0') break;
			if(arquivo[i] > '9') break;
			i++;
			if(i>8) break;
		}	
		arquivo[i] = 0;	//Terminador
		//Converte para inteiro
		num_arquivo = StringToInt(arquivo);
		
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		UART0_TxString("Digite T para TXT ou B para Bitmap: ");
		//Lê um char com o tipo
		tipo = UART0_RxChar();
		if((tipo != 'T') && (tipo != 't') && (tipo != 'B') && (tipo != 'b'))
			continue;

		//Seleciona o arquivo (1o cluster)
		next_cluster=SD_get_first_cluster(num_arquivo);
		
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);

		
		//Mostra o arquivo TXT
		if((tipo == 'T') || (tipo == 't'))
		{		
			UART0_TxString("Conteudo do arquivo lido: "); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
			UART0_TxChar(0x0D); UART0_TxChar(0x0A);
			do
			{
				next_cluster=SD_open_file_TXT(next_cluster,SSI_2);
			}
			while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
		}
	
		//Mostra o arquivo BMP
//		if((tipo == 'B') || (tipo == 'b'))
//		next_cluster=SD_open_file_BMP(next_cluster, 1, SSI_2);	//1o setor do 1o cluster (cabeçalho)
//		if(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF)
//		{
//			UART0_TxString("Conteudo do arquivo lido: "); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//			UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//			do
//			{
//				next_cluster=SD_open_file_BMP(next_cluster, 0, SSI_2);	//Demais clusters
//			}
//			while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
//		}
		if((tipo == 'B') || (tipo == 'b'))
		{
			TransfBitmatFromSDCardToLCD(num_arquivo);
			UART0_TxString("Conteudo do arquivo lido transferido para o LCD TFT do MKII"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
		}
		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
	}
	
//	while(1)
//	{
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("========> File to be read <========"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("Digite o nome do arquivo a ser lido: ");
//		
//		//Lê uma string até o enter ou qualquer caractere não numérico
//		i=0;
//		for(;;)
//		{
//			arquivo[i] = UART0_RxChar();
//			//UART0_TxChar(arquivo[i]);		//Eco
//			if(arquivo[i] < ' ') break;
//			if(arquivo[i] > 'z') break;
//			i++;
//			if(i>254) break;
//		}		
//		arquivo[i] = 0;	//Terminador
//		
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("Arquivo: "); UART0_TxString(arquivo); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("Digite T para TXT ou B para Bitmap: ");
//		//Lê um char com o tipo
//		tipo = UART0_RxChar();
//		if((tipo != 'T') && (tipo != 't') && (tipo != 'B') && (tipo != 'b'))
//			continue;

//		//Seleciona o número do arquivo
//		num_arquivo=SD_get_number_name(arquivo);
//		
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("Number of file: "); IntToString(num_arquivo, arquivo); UART0_TxString(arquivo); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		
//		//Seleciona o arquivo (1o cluster)
//		next_cluster=SD_get_first_cluster(num_arquivo);
//		
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxString("First cluster: "); IntToString(next_cluster, arquivo); UART0_TxString(arquivo); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);

//		//Se é um arquivo inválido
//		if(num_arquivo >= 40) continue;
//		
//		//Mostra o arquivo TXT
//		if((tipo == 'T') || (tipo == 't'))
//		{		
//			UART0_TxString("Conteudo do arquivo lido: "); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//			UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//			do
//			{
//				next_cluster=SD_open_file_TXT(next_cluster,SSI_2);
//			}
//			while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
//		}
//	
//		//Mostra o arquivo BMP
////		if((tipo == 'B') || (tipo == 'b'))
////		next_cluster=SD_open_file_BMP(next_cluster, 1, SSI_2);	//1o setor do 1o cluster (cabeçalho)
////		if(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF)
////		{
////			UART0_TxString("Conteudo do arquivo lido: "); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
////			UART0_TxChar(0x0D); UART0_TxChar(0x0A);
////			do
////			{
////				next_cluster=SD_open_file_BMP(next_cluster, 0, SSI_2);	//Demais clusters
////			}
////			while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
////		}
//		if((tipo == 'B') || (tipo == 'b'))
//		{
//			TransfBitmatFromSDCardToLCD(num_arquivo);
//			UART0_TxString("Conteudo do arquivo lido transferido para o LCD TFT do MKII"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		}
//		UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//	}
	
	while(1);
	
	return 0;
}
//*/

//Função principal Buzzer
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t temp, umi, i;
	uint8_t nota = 0;

	//Setup
	//PLL_Inicializa();
	
	//GPIOF_Inicializa();
	LedRed_Inicializa();
	
	//UART0_Inicializa_115200bps_80MHz();
	//UART0_Inicializa_115200bps_16MHz();
	
	//SysTick_Inicializa();
	//SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intersecamente)
	
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_LCD_DrawBitmap(34, 84, mario_60x40, 60, 40);

	Timer0_Init();

	MKII_Timer1_PWM_Init(1000, 50);
	//MKII_Timer1_PWM_Set(10000, 50);

	//Ordem das notas	1ª		2ª			3ª		4ª			5ª		6ª		7ª				8ª	9ª				10ª	11ª			12ª		1ª...
	//Nomes das notas	Dó		Dó#/Réb	Ré		Ré#/Mib	Mi		Fá		Fá#/Solb	Sol	Sol#/Láb	Lá	Lá#/Sib	Si		Dó
	//Frequências	    261,6	277			293,5	311			329,6	349,2	370				392	415,1			440	466,2		493,8	523,2
	while(1)
	{
		switch(nota)
		{
			case 0: //Dó
				MKII_Timer1_PWM_Set(261, 10);	break;
			case 1: //Ré
				MKII_Timer1_PWM_Set(293, 10);	break;
			case 2: //Mi
				MKII_Timer1_PWM_Set(329, 10);	break;
			case 3: //Fá
				MKII_Timer1_PWM_Set(349, 10);	break;
			case 4: //Sol
				MKII_Timer1_PWM_Set(392, 10);	break;
			case 5: //Lá
				MKII_Timer1_PWM_Set(440, 5);	break;		//Lá tem ganho muito maior no buzzer
			case 6: //Si
				MKII_Timer1_PWM_Set(493, 10);	break;
			case 7: //Dó
				MKII_Timer1_PWM_Set(523, 10);	break;
		}
		nota = ++nota%8;
		
		while(msTicks <= 200);
		msTicks = 0;
		GPIOF->DATA = GPIOF->DATA ^ Red;
	}
}
//*/
//Função principal Buzzer com Mario Tunes
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t temp, umi, i;
	uint8_t nota = 0;

	//Setup
	//PLL_Inicializa();
	
	//GPIOF_Inicializa();
	LedRed_Inicializa();
	
	//UART0_Inicializa_115200bps_80MHz();
	//UART0_Inicializa_115200bps_16MHz();
	
	//SysTick_Inicializa();
	//SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intersecamente)
	
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_LCD_DrawBitmap(34, 84, mario_60x40, 60, 40);
	BSP_Button1_Init();
	BSP_Button2_Init();

	Timer0_Init();	//Gera uma interrupção a cada 1 ms

	MKII_Timer1_PWM_Init(1000, 50);
	TIMER1->CTL &= ~(1<<0);		 				//Desabilita Timer1A e começa a contar (bit 0 - TAEN)
	//MKII_Timer1_PWM_Set(10000, 50);

	//Ordem das notas	1ª		2ª			3ª		4ª			5ª		6ª		7ª				8ª	9ª				10ª	11ª			12ª		1ª...
	//Nomes das notas	Dó		Dó#/Réb	Ré		Ré#/Mib	Mi		Fá		Fá#/Solb	Sol	Sol#/Láb	Lá	Lá#/Sib	Si		Dó
	//Frequências	    261,6	277			293,5	311			329,6	349,2	370				392	415,1			440	466,2		493,8	523,2
	while(1)
	{
		if(!BSP_Button1_Input())
		{
			BSP_LCD_SetCursor(1, 10); BSP_LCD_OutUDec(sizeof(theme_melody)/sizeof(int), LCD_YELLOW);
			
			GPIOF->DATA = GPIOF->DATA | Red;
			TIMER1->CTL |= 1<<0;											//Habilita Timer1A
			play_tunes(theme_melody, theme_tempo, sizeof(theme_melody)/sizeof(int), 1);
			GPIOF->DATA = GPIOF->DATA & ~Red;
			TIMER1->CTL &= ~(1<<0);										//Desabilita Timer1A
		}
		if(!BSP_Button2_Input())
		{			
			BSP_LCD_SetCursor(1, 10); BSP_LCD_OutUDec(sizeof(underworld_melody)/sizeof(int), LCD_YELLOW);
			
			GPIOF->DATA = GPIOF->DATA | Red;
			TIMER1->CTL |= 1<<0;
			play_tunes(underworld_melody, underworld_tempo, sizeof(underworld_melody)/sizeof(int), 1);
			GPIOF->DATA = GPIOF->DATA & ~Red;
			TIMER1->CTL &= ~(1<<0);
		}
	}
}
//*/

//Função principal Buzzer com Mario Tunes & Divertidance
/*
#define diverti
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t temp, umi, i;
	uint8_t nota = 0;
	//bool lado = 0;

	//Setup
	//PLL_Inicializa();
	
	//GPIOF_Inicializa();
	LedRed_Inicializa();
	
	//UART0_Inicializa_115200bps_80MHz();
	//UART0_Inicializa_115200bps_16MHz();
	
	//SysTick_Inicializa();
	//SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intersecamente)
	
	Timer0_Init();	//Gera uma interrupção a cada 1 ms
	
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	
	BSP_LCD_DrawBitmap(0, 63, divertidance_64x64, 64, 64);
	msTicks = 0; while(msTicks<200);
	BSP_LCD_DrawBitmap(64, 127, divertidance_64x64, 64, 64);
	msTicks = 0; while(msTicks<200);
	BSP_LCD_DrawBitmap(64, 63, divertidance_64x64, 64, 64);
	msTicks = 0; while(msTicks<200);
	BSP_LCD_DrawBitmap(0, 127, divertidance_64x64, 64, 64);
	msTicks = 0; while(msTicks<1000);
	
	BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_LCD_DrawBitmap(32, 63, divertidance_64x64, 64, 64);
	
	msTicks = 0; while(msTicks<500);
	BSP_LCD_DrawBitmap(10, 123, palito1_24x46, 24, 46);
	msTicks = 0; while(msTicks<200);
	BSP_LCD_DrawBitmap(52, 123, palito1_24x46, 24, 46);
	msTicks = 0; while(msTicks<200);
	BSP_LCD_DrawBitmap(93, 123, palito1_24x46, 24, 46);
	lado = 0;
	
		
	BSP_Button1_Init();
	BSP_Button2_Init();

	MKII_Timer1_PWM_Init(1000, 50);
	TIMER1->CTL &= ~(1<<0);		 				//Desabilita Timer1A e começa a contar (bit 0 - TAEN)
	//MKII_Timer1_PWM_Set(10000, 50);

	//Ordem das notas	1ª		2ª			3ª		4ª			5ª		6ª		7ª				8ª	9ª				10ª	11ª			12ª		1ª...
	//Nomes das notas	Dó		Dó#/Réb	Ré		Ré#/Mib	Mi		Fá		Fá#/Solb	Sol	Sol#/Láb	Lá	Lá#/Sib	Si		Dó
	//Frequências	    261,6	277			293,5	311			329,6	349,2	370				392	415,1			440	466,2		493,8	523,2
	while(1)
	{
		if(!BSP_Button1_Input())
		{
//			BSP_LCD_SetCursor(1, 10); BSP_LCD_OutUDec(sizeof(theme_melody)/sizeof(int), LCD_YELLOW);
			
			GPIOF->DATA = GPIOF->DATA | Red;
			TIMER1->CTL |= 1<<0;											//Habilita Timer1A
			play_tunes(theme_melody, theme_tempo, sizeof(theme_melody)/sizeof(int), 1);
			GPIOF->DATA = GPIOF->DATA & ~Red;
			TIMER1->CTL &= ~(1<<0);										//Desabilita Timer1A
		}
		if(!BSP_Button2_Input())
		{			
//			BSP_LCD_SetCursor(1, 10); BSP_LCD_OutUDec(sizeof(underworld_melody)/sizeof(int), LCD_YELLOW);
			
			GPIOF->DATA = GPIOF->DATA | Red;
			TIMER1->CTL |= 1<<0;
			play_tunes(underworld_melody, underworld_tempo, sizeof(underworld_melody)/sizeof(int), 1);
			GPIOF->DATA = GPIOF->DATA & ~Red;
			TIMER1->CTL &= ~(1<<0);
		}
	}
}
//*/
//Função principal Aula 3 - Timer com PWM (Servo e Buzzer) - 30/03/19
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t x,y,click, angulo;
	uint8_t nota = 0;
	//bool lado = 0;
	int delay;

	//Setup
	LedRed_Inicializa();
	UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intrinsecamente)
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_Joystick_Init();
	MKII_Servo_Init();
		
	//MKII_Timer1_PWM_Init(1000, 50);
	//TIMER1->CTL &= ~(1<<0);		 				//Desabilita Timer1A e começa a contar (bit 0 - TAEN)

	//Ordem das notas	1ª		2ª			3ª		4ª			5ª		6ª		7ª				8ª	9ª				10ª	11ª			12ª		1ª...
	//Nomes das notas	Dó		Dó#/Réb	Ré		Ré#/Mib	Mi		Fá		Fá#/Solb	Sol	Sol#/Láb	Lá	Lá#/Sib	Si		Dó
	//Frequências	    261,6	277			293,5	311			329,6	349,2	370				392	415,1			440	466,2		493,8	523,2
	
	BSP_LCD_DrawString(0, 5, "CAD    ", LCD_WHITE);
	BSP_LCD_DrawString(0, 7, "ANGULO ", LCD_WHITE);
	
	while(1)
	{
		BSP_Joystick_Input(&x, &y, &click);
		
		//(1023-0)/(90-90) = 5,68 ~ 6
		//512 = 0 graus
		//0 = -90 graus
		//1023 = 90 graus
		angulo = (x/6) - 90;
		MKII_Servo_Set_Angle(angulo);
		
		BSP_LCD_SetCursor(8, 5);
		BSP_LCD_OutUDec4(x, LCD_WHITE);
		BSP_LCD_SetCursor(8, 7);
		if(angulo >= 0)
		{
			BSP_LCD_OutUDec4(angulo, LCD_WHITE);
		}
		else
		{
			BSP_LCD_OutUDec4(-angulo, LCD_WHITE);
			BSP_LCD_DrawString(8, 7, "-", LCD_WHITE);
		}
		delay = 0;
		while(delay++<100000);
	}
}
//*/

//Função principal Teste Acelerômetro KXTC9-2050
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t x,y,z,click, angulo;
	uint8_t nota = 0;
	//bool lado = 0;
	int delay;

	//Setup
	LedRed_Inicializa();
	UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intrinsecamente)
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_Joystick_Init();
	BSP_Accelerometer_Init();
	BSP_Accelerometer_Input(&x, &y, &z);
	
	BSP_LCD_DrawString(0, 0, "Acel. KXTC9-2050", LCD_WHITE);
	BSP_LCD_DrawString(0, 1, "X ", LCD_WHITE);
	BSP_LCD_DrawString(0, 2, "Y ", LCD_WHITE);
	BSP_LCD_DrawString(0, 3, "Z ", LCD_WHITE);
	
	while(1)
	{
		BSP_Accelerometer_Input(&x, &y, &z);
		BSP_LCD_SetCursor(2, 1);
		BSP_LCD_OutUDec4(x, LCD_WHITE);
		BSP_LCD_SetCursor(2, 2);
		BSP_LCD_OutUDec4(y, LCD_WHITE);
		BSP_LCD_SetCursor(2, 3);
		BSP_LCD_OutUDec4(z, LCD_WHITE);
		
		delay = 0;
		while(delay++<100000);
	}
}
//*/

//Função principal Nível Digital com o Acelerômetro KXTC9-2050
/*
int main()
{
	//Variáveis locais
	unsigned char dado;
	int16_t x,y,z,xn,yn,xn_ant,yn_ant,xg,yg;
	uint8_t nota = 0;
	//bool lado = 0;
	int delay;

	//Setup
	LedRed_Inicializa();
	UART0_Inicializa_115200bps_16MHz();
	SysTick_Inicializa();
	SysTick_Config(SystemCoreClock / 1000);      // 1 int./ms (interrupção configurada intrinsecamente)
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0xFF, 0xFF, 0xFF));
	BSP_Joystick_Init();
	BSP_Accelerometer_Init();
	BSP_Accelerometer_Input(&x, &y, &z);
	

	BSP_LCD_FillScreen(LCD_BLACK);
	BSP_LCD_FillCircle(64, 64, 60, LCD_WHITE);
	BSP_LCD_DrawFastHLine(4, 64, 120, LCD_YELLOW);
	BSP_LCD_DrawFastVLine(64, 4, 120, LCD_YELLOW);

	BSP_LCD_DrawString(0, 0, "Nivel Digital XY", LCD_WHITE);
	BSP_LCD_DrawString(0, 1, "X ", LCD_WHITE);
	BSP_LCD_DrawString(0, 2, "Y ", LCD_WHITE);
	
	xn = yn = xn_ant = yn_ant = 64;
	BSP_LCD_FillCircle(xn, yn, 10, LCD_RED);
	
	// Sensibilidade de 660 mV/g --> CAD = 204,6 ~ 205
	// Offset a 0g de 1,65 V --> CAD = 512
	//
	// CAD:  512 + 205 <= X <= 512 - 205
	// Tela: 20        <= X <= 108
	// DrawX = a.CAD + b, onde a = (20-108)/((512+205)-(512-205)) = -88/410 = -0,2146...
	//										onde b = 108 - (-88/410) * 307 = 173,8926 ~ 174
	// CAD:  512 - 205 <= Y <= 512 + 205
	// Tela: 20        <= Y <= 108
	// DrawY = a.CAD + b, onde a = (108-20)/((512+205)-(512-205)) = 88/410 = 0,2146...
	//										onde b = 20 - (88/410) * 307 = -45,8926 ~ -46
	//
	// DrawG = a.CAD + b, onde a = (1-(-1))/((512+205)-(512-205)) = 2/410 = 1/205
	//                    onde b = -1/205 * 512 = -2,4975 ~ -2,5
	
	while(1)
	{
		//Lê o acelerômetro
		BSP_Accelerometer_Input(&x, &y, &z);
		//Calcula a nova posição do nível		
		xn = -(88 * x) / 410 + 174;
		yn = (88 * y) / 410 - 46;
		//Teste para ver se houve alteração
		if((xn != xn_ant) || (yn != yn_ant))
		{
			//Apaga o circulo anterior
			BSP_LCD_FillCircle(xn_ant, yn_ant, 10, LCD_WHITE);
			//Redesenha os eixos
			BSP_LCD_DrawFastHLine(4, 64, 120, LCD_YELLOW);
			BSP_LCD_DrawFastVLine(64, 4, 120, LCD_YELLOW);
			//Calcula a força g nos eixos
			xg = (10*x) /205 - 25;	//Valor x10
			yg = (10*y) /205 - 25;	//Valor x10
			//Atualiza os valores
			BSP_LCD_SetCursor(2, 1);
			//BSP_LCD_OutUDec4(x, LCD_WHITE);
			if(xg>=0)
				BSP_LCD_OutUFix2_1(xg, LCD_WHITE);
			else
			{
				BSP_LCD_OutUFix2_1(-xg, LCD_WHITE);
				BSP_LCD_DrawString(2, 1, "-", LCD_WHITE);
			}
			BSP_LCD_SetCursor(2, 2);
			//BSP_LCD_OutUDec4(y, LCD_WHITE);
			if(yg>=0)
				BSP_LCD_OutUFix2_1(yg, LCD_WHITE);
			else
			{
				BSP_LCD_OutUFix2_1(-yg, LCD_WHITE);
				BSP_LCD_DrawString(2, 2, "-", LCD_WHITE);
			}
			//Atualiza o circulo
			BSP_LCD_FillCircle(xn, yn, 10, LCD_RED);
		}
		//Salva o valor anterior
		xn_ant = xn;
		yn_ant = yn;		
		//Aguarda alguns ms
		msTicks = 0;
		while(msTicks<50);
	}
}
//*/

//Função principal Máquina de Estados
//*
//Representa um estado da FSM 
struct State {
      unsigned char 	out;   	//Saída para o estado
      unsigned short 	wait;     	//Tempo de espera do estado
      unsigned char 	next[2]; 	//Vetor de próximos estados
};
typedef const struct State tipoS;
//Apelidos para referenciar os estados da FSM
#define Par 0
#define Impar 1
//Estrutura de dados que corresponde ao diagrama de transição de estados da FSM
tipoS Fsm[2] = { 
      {0, 100, {Par, Impar}},
      {1, 100, {Impar, Par}}
}; 
unsigned char cState; 		//Estado atual é Par ou Ímpar
int main(void) 
{
	unsigned char input;
	GPIOF_Inicializa();
	SysTick_Inicializa();
	cState = Par; 		//Initial State
	while (1) 
	{
		//1. Saída baseado no estado atual
		GPIOPinWrite(GPIOF_BASE, GPIO_PIN_2, 0xFF); 	//Saída em PF2 (led Azul)
		//2. Aguarda o tempo predefinido para o estado
		SysTick_Wait10ms(Fsm[cState].wait);   
		//3. Lê a entrada
		input = GPIOPinRead(GPIOF_BASE, GPIO_PIN_4)>>4;	//Entrada 0/1: Sw1 Pres./Não Pres.
		//4. Vai para o próximo estado, que depende da entrada e do estado atual
		cState = Fsm[cState].next[input]; 
	}
}
//*/
//*****************************************************************************
//	Funções Auxiliares
//*****************************************************************************

//Envia uma string para o LCD TFT
//Recebe: caractere de 8 bits ASCII
void LCD_TxString(unsigned char* data)
{
	static unsigned int tamanho = 0;
	static unsigned char sobrou[3] = {0,0,0};
	unsigned int i = 0;
	unsigned int setor = 512;
	unsigned short int cor;
	unsigned char tam[11];
	
	//Desabilita o SD Card **************************
	SD_cs_high(SSI_2);
	
	
	//Se é o primeiro setor do arquivo, possui o cabeçalho
	if(BitmapStart)
	{
		BitmapStart = 0;
		//Pula o cabeçalho
		i = 54;
		//Lê o tamanho da área de dados do arquivo em bytes
		tamanho = data[0x22] + (unsigned int)(data[0x23]<<8) + (unsigned int)(data[0x24]<<16) + (unsigned int)(data[0x25]<<24);
	
//Depuração
//		UART0_TxString("Depura LCD_TxString 0123456789: ");
//EndDeuração
		//TODO: Aqui seria um bom lugar para definir o tamanho da imagem no LCD caso se deseje ela variável.
		//Para isso usar o comando setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
		//Talvez criar duas variáveis globais para determinar x0 e y0 e ler os seguintes bytes do cabeçalho
		//do arquivo para definir y1 = w (byte 18) e x1 = h (byte 22).
		//Para arquivos com tamanho fixo 128 x 128, essa função pode ser chamada na função
		//TransfBitmatFromSDCardToLCD(unsigned int num_arquivo).
		//A função setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) precisa ser passada de 
		//static para public na biblioteca BSP.
	}

//Depuração
//	UART0_TxString(data); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//		return;	
//	UART0_TxString("Tamanho: "); IntToString(tamanho, tam);    UART0_TxString(tam); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//	UART0_TxString("i      : "); IntToString(i, tam);          UART0_TxString(tam); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//	UART0_TxString("sobrou : "); IntToString(sobrou[0], tam);  UART0_TxString(tam); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//EndDeuração
	
	//Envia os pixels enquanto não acabar o setor ou o Bitmap
	while((i <= (512-3)) && (tamanho >= 3))	//24 bits por pixels
	{
		if(sobrou[0] == 0)
		{
			//((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			//Seguencia BGR (24 bits) --> RGB (565)
			cor = (data[i] >> 3) | ((data[i+1] & 0xFC) << 3) | ((data[i+2] & 0xF8) << 8);
			i += 3;
			tamanho -= 3;
		}
		else if(sobrou[0] == 1)
		{
			//Sobrou a cor Azul
			cor = (sobrou[2] >> 3) | ((data[i] & 0xFC) << 3) | ((data[i+1] & 0xF8) << 8);
			i += 2;
			tamanho -= 2;
		}
		else if(sobrou[0] == 2)
		{
			//Sobrou a cor Azul e Verde
			cor = (sobrou[1] >> 3) | ((sobrou[2] & 0xFC) << 3) | ((data[i] & 0xF8) << 8);
			i += 1;
			tamanho -= 1;
		}
		else
		{
			i = 512;
			setor = 0;
			tamanho = 0;
			break;
		}
		//Envia pixel 565 para o LCD
		//Envia o MSB
		writedata(cor>>8);
		//Envia o LSB
		writedata(cor);		
		//OK: Essa função é static na lib BSP passar para public

//Depuração
//		UART0_TxChar(cor>>8);
//		UART0_TxChar(cor);
//		UART0_TxChar('.');
//EndDeuração
		
		sobrou[0] = 0;	//Sobra algum byte apenas no final do setor (i>= 510)
	}
	//Se ainda não acabou o arquivo
	if(tamanho > 0)
	{
		//Salva o número de bytes que sobreram para formar um pixels
		sobrou[0] = 512 - i;
		//Completa os 512 bytes do setor
		tamanho -= sobrou[0];
		//Salva o penúltimo byte
		sobrou[1] = data[510];
		//Salva o último byte
		sobrou[2] = data[511];
	}
	else
	{
//Depuração	
//		UART0_TxString("xxxxxx Fim LCD_TxString xxxxxx"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//EndDepuração	
	}
//Depuração	
//	UART0_TxString("xxxxxx LCD_TxString xxxxxx"); UART0_TxChar(0x0D); UART0_TxChar(0x0A);
//	UART0_TxString(data);
//EndDepuração
	
	//Habilita o SD Card **************************
	SD_cs_low(SSI_2);
}

//Transfere Bitmap do SD Card para o LCD
unsigned int TransfBitmatFromSDCardToLCD(unsigned int num_arquivo)
{
	//Variáveis locais
	long next_cluster;
	
	//Define as funções para envio de mensagens do SD Card
	SD_define_print_function(LCD_TxString, UART0_TxChar);
	
	//Desabilita o LDC e o SDCard
//	FT_CS = TFT_CS_HIGH;
	SD_cs_high(SSI_2); //GPIO_PORTB_DATA_R|=0x20;	//OK: Esse pino é usado pelo Joystick (TROCAR)
	
	//Ajusta o tamanho e coordenadas da área utilizada do LDC (x0,y0) (x1,y1)
	//TODO: passar a função de static para public
//	setAddrWindow(0, 0, 127, 127);
	//OK: Checar se a função ativa o sinal de enable do LCD (todo escrita faz isso)
	//Os próximos 128x128x2 bytes enviados ao LCD serão para representar o Bitmap vindo do SDCard
	
	//Seleciona o arquivo (1o cluster)
	next_cluster=SD_get_first_cluster(num_arquivo);
	//OK: Checar se a função ativa e desativa o sinal de enable do SD Card (isso já está na estrutura de dados)
	
	//Seta a variável global de sinalização da início de Bitmap
	BitmapStart = 1;
	
	//Lê o número do primero setor do SC Card
	next_cluster=SD_open_file_BMP_raw(next_cluster, 1, SSI_2);	//1o setor do 1o cluster (cabeçalho)
	//OK: Checar se a função ativa e desativa o sinal de enable do SD Card
	
	if(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF)
	{
		do
		{
			next_cluster=SD_open_file_BMP_raw(next_cluster, 0, SSI_2);	//Demais clusters
			//OK: Checar se a função ativa e desativa o sinal de enable do SD Card
		}
		while(next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
	}
	
	//Define as funções para envio de mensagens do SD Card
	SD_define_print_function(UART0_TxString, UART0_TxChar);
}

//Toca música de acordo com a frequência, tempo de cada nota e o número de repetições
void play_tunes(const int tune[],const int tempo[], int tamanho, int repete)
{
	int i, j;
	static int nota = 0;
	
//	BSP_LCD_SetCursor(1, 12); BSP_LCD_OutUDec(tamanho, LCD_YELLOW);
	for(i=0; i<repete; i++)
	{
		for(j=0; j<tamanho; j++)
		{
			if(tune[j] != 0)
			{
				TIMER1->CTL |= 1<<0;											//Habilita Timer1A
				MKII_Timer1_PWM_Set(tune[j], 50);
			}
			else
			{
				TIMER1->CTL &= ~(1<<0);										//Desabilita Timer1A
			}
			
			msTicks = 0;
#ifdef diverti
			if(nota == 2)
			{
				nota = 0;
				lado = !lado;
				if(lado == 0)
				{
					BSP_LCD_DrawBitmap(10, 123, palito1_24x46, 24, 46);
					BSP_LCD_DrawBitmap(52, 123, palito1_24x46, 24, 46);
					BSP_LCD_DrawBitmap(93, 123, palito1_24x46, 24, 46);
				}
				else
				{
					BSP_LCD_DrawBitmap(10, 123, palito2_24x46, 24, 46);
					BSP_LCD_DrawBitmap(52, 123, palito2_24x46, 24, 46);
					BSP_LCD_DrawBitmap(93, 123, palito2_24x46, 24, 46);
				}
			}
			nota++;
#endif
			
			while(msTicks < (tempo[j]*10));
		}
	}	
}
