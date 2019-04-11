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
 */

#include "inc/tm4c123gh6pm.h"
#include <stdint.h>
//Leandro (17/02/2019) - para dar acesso as funções criadas no arquivo main (exemplo: UART0_TxString)
//#include "main.c"


/* Definitions for MMC/SDC commands */
#define CMD0     0x40    	/* GO_IDLE_STATE */
#define CMD1     0x41    	/* SEND_OP_COND */
#define CMD8     0x48    	/* SEND_IF_COND */
#define CMD9     0x49    	/* SEND_CSD */
#define CMD10    0x4A   	/* SEND_CID */
#define CMD12    0x4C    	/* STOP_TRANSMISSION */
#define CMD16    0x50    	/* SET_BLOCKLEN */
#define CMD17    0x51    	/* READ_SINGLE_BLOCK */
#define CMD18    0x52    	/* READ_MULTIPLE_BLOCK */
#define CMD23    0x57    	/* SET_BLOCK_COUNT */
#define CMD24    0x58    	/* WRITE_BLOCK */
#define CMD25    0x59    	/* WRITE_MULTIPLE_BLOCK */
#define CMD41    0x69    	/* SEND_OP_COND (ACMD) */
#define CMD55    0x77    	/* APP_CMD */
#define CMD58    0x7A    	/* READ_OCR */

enum typeOfWrite{
  COMMAND,                              // the transmission is an LCD command
  DATA                                  // the transmission is data
};

//enum SSI{
//	SSI0,
//	SSI1,
//	SSI2,
//	SSI3
//};
//Leandro (17/02/2019) - precisei alterar para evitar conflito com CMSIS
enum SSI{
	SSI_0,
	SSI_1,
	SSI_2,
	SSI_3
};

enum name_type{
	SHORT_NAME,
	LONG_NAME
};

enum get_subdirs{
	GET_SUBDIRS,
	NO_SUBDIRS
};

//Leandro (28/02/2019): Tipo criado para envio de mensagens
typedef void TipoFuncaoString(unsigned char*);
typedef void TipoFuncaoChar(unsigned char);
//Leandro (28/02/2019): Macros
#define printEnter()	printc(0x0D);printc(0x0A)
#define printTab()		printc(0x09)

void startSSI0(void);
void startSSI1(void);
void SD_startSSI2(void);
void startSSI3(void);
void sd_write(char message,enum SSI);
unsigned char sd_read(enum SSI);
unsigned char SD_initialize_sd(enum SSI);
void SD_cs_high(enum SSI);
void SD_cs_low(enum SSI);
void dummy_clock(enum SSI);
void tx_high(enum SSI);
void SD_Timer5_Init(void);
void SD_change_speed(enum SSI);
long SD_open_file_TXT(long next_cluster,enum SSI);
long SD_get_root_dir_first_cluster(void);
long SD_get_first_cluster(int pos);
long SD_list_dirs_and_files(long next_cluster,enum name_type name, enum get_subdirs subdirs, enum SSI SSI_number);
void tx_SSI(enum SSI);
void clean_name(void);
void disk_timerproc(void);
//Leandro (17/02/2019) - Faltou o protótipo destas funções
void SD_read_first_sector(enum SSI SSI_number);
void SD_read_disk_data(enum SSI SSI_number);
unsigned int rcvr_datablock (unsigned char *buff, unsigned int btr, enum SSI SSI_number);
void rcvr_spi_m(unsigned char *dst,enum SSI SSI_number);
//Leandro (26/02/2019): Função criada para depuração
void show_MBR(unsigned char boot[]);
//Leandro (28/02/2019): Função criada para envio de mensagens
void SD_define_print_function(TipoFuncaoString *PonteiroParaUmaFuncaoString, TipoFuncaoChar *PonteiroParaUmaFuncaoChar);
//Leandro (28/02/2019): Conversão de inteiro para string decimal
static void SD_IntToString(unsigned int n, unsigned char* s);
//Leandro (28/02/2019): Conversão de inteiro para string hexa
static void SD_IntToStringHexa(unsigned int n, unsigned char* s);
//Leandro (28/02/2019): Conversão de string para inteiro
static unsigned int SD_StringToInt(unsigned char* s);
//Leandro (28/02/2019): Abre um arquivo do tipo BMP
long SD_open_file_BMP(long next_cluster, int first, enum SSI SSI_number);
//Leandro (28/02/2019): Mostra todo conteúdo do cabeçalho de um arquivo BMP
void show_BMP_Header(unsigned char header[]);
//Leandro (04/03/2019): Abre um arquivo do tipo BMP crú e envia pela função de transferência de strings
long SD_open_file_BMP_raw(long next_cluster, int first, enum SSI SSI_number);
//Leandro (05/03/2019): Solicitar o primeiro cluster de um arquivo através de seu nome
long SD_get_first_cluster_name(unsigned char* name);
//Leandro (05/03/2019): Solicitar o número do arquivo através de seu nome
long SD_get_number_name(unsigned char* name);
