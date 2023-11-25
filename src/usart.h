//---------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### USART.H ###################
//---------------------------------
#ifndef __USART_H_
#define __USART_H_

// Module Exported Types Constants and Macros ----------------------------------
// #define USARTx_RX_DISA	(USARTx->CR1 &= 0xfffffffb)
// #define USARTx_RX_ENA	(USARTx->CR1 |= 0x04)
#define USART_9600    5000
#define USART_115200    416
#define USART_250000    192

#define SIZEOF_DATA 128
#define BUFFRX_DIM SIZEOF_DATA
#define BUFFTX_DIM SIZEOF_DATA

#define UsartRxBinary()    Usart1RxBinary()

//--- Exported functions ---//
unsigned char Usart1ReadBuffer (unsigned char *, unsigned short);
unsigned char Usart2ReadBuffer (unsigned char *, unsigned short);

void Usart1Config (void);
void USART1_IRQHandler (void);
void Usart1Send (char *);
void Usart1SendUnsigned(unsigned char *, unsigned char);
unsigned char Usart1HaveData (void);
void Usart1HaveDataReset (void);
void Usart1RxBinary (void);
void Usart1ChangeBaud (unsigned short new_baud);

void Usart2Config (void);
void USART2_IRQHandler (void);
void Usart2Send (char *);
void Usart2SendUnsigned(unsigned char *, unsigned char);

#endif    /* __USART_H_ */

//--- end of file ---//
