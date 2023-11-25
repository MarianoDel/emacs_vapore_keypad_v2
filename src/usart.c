//---------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### USART.C ###################
//---------------------------------

// Includes --------------------------------------------------------------------
#include "hard.h"
#include "stm32f0xx.h"
#include "usart.h"

#include "flash_program.h"
#include "memory_conf.h"

#include <string.h>

#define USART1_IN_USE
#define USART2_IN_USE


// Module Private Types Constants and Macros -----------------------------------
#define USART1_CLK    (RCC->APB2ENR & 0x00004000)
#define USART1_CLK_ON    (RCC->APB2ENR |= 0x00004000)
#define USART1_CLK_OFF    (RCC->APB2ENR &= ~0x00004000)

#define USART2_CLK    (RCC->APB1ENR & 0x00020000)
#define USART2_CLK_ON    (RCC->APB1ENR |= 0x00020000)
#define USART2_CLK_OFF    (RCC->APB1ENR &= ~0x00020000)


// Externals -------------------------------------------------------------------
extern mem_bkp_typedef memory_backup;
extern volatile unsigned char binary_full;
// extern volatile unsigned short binary_bytes;



// Globals ---------------------------------------------------------------------
#ifdef USART1_IN_USE
volatile unsigned char usart1_have_data = 0;
volatile unsigned char * ptx1;
volatile unsigned char * ptx1_pckt_index;
volatile unsigned char * prx1;
volatile unsigned char tx1buff[SIZEOF_DATA];
volatile unsigned char rx1buff[SIZEOF_DATA];
#endif

#ifdef USART2_IN_USE
volatile unsigned char usart2_have_data = 0;
volatile unsigned char * ptx2;
volatile unsigned char * ptx2_pckt_index;
volatile unsigned char * prx2;
volatile unsigned char tx2buff[SIZEOF_DATA];
volatile unsigned char rx2buff[SIZEOF_DATA];
#endif

volatile unsigned short binary_bytes = 1024;

// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
//////////////////////
// Usart1 Functions //
//////////////////////
#ifdef USART1_IN_USE
void Usart1Config (void)
{
    if (!USART1_CLK)
        USART1_CLK_ON;

    USART1->BRR = USART_9600;
    // USART1->CR2 |= USART_CR2_STOP_1;    //2 bits stop
    // USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
    // USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;	// no Tx
    USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;    // Rx & Tx
    
    ptx1 = tx1buff;
    ptx1_pckt_index = tx1buff;
    prx1 = rx1buff;

    // Set alternative pin functions
    unsigned int temp;
    temp = GPIOA->AFR[1];
    temp &= 0xFFFFF00F;
    temp |= 0x00000110;    //PA10 -> AF1 PA9 -> AF1
    GPIOA->AFR[1] = temp;
    
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 5);
}


unsigned char Usart1ReadBuffer (unsigned char * bout, unsigned short max_len)
{
    unsigned int len;

    len = prx1 - rx1buff;

    if (len < max_len)
    {
        *prx1 = '\0';    //buffer from int isnt ended with '\0' do it now
        len += 1;    //space for '\0'
    }
    else
    {
        *(bout + max_len - 1) = '\0';
        len = max_len - 1;
    }

    memcpy(bout, (unsigned char *) rx1buff, len);

    //pointer adjust after copy
    prx1 = rx1buff;
    return (unsigned char) len;
}


void Usart1Send (char * send)
{
    unsigned char i;

    i = strlen(send);
    Usart1SendUnsigned((unsigned char *)send, i);
}


void Usart1SendUnsigned(unsigned char * send, unsigned char size)
{
    if ((ptx1_pckt_index + size) < &tx1buff[SIZEOF_DATA])
    {
        memcpy((unsigned char *)ptx1_pckt_index, send, size);
        ptx1_pckt_index += size;
        USART1->CR1 |= USART_CR1_TXEIE;
    }
}


unsigned char Usart1HaveData (void)
{
    return usart1_have_data;
}


void Usart1HaveDataReset (void)
{
    usart1_have_data = 0;
}


void USART1_IRQHandler(void)
{
    unsigned char dummy;

    // USART in Receiv Mode ------------------------------------------
    if (USART1->ISR & USART_ISR_RXNE)
    {
        dummy = USART1->RDR & 0x0FF;

        //formato binario
        if (binary_bytes < 1024)
        {
            memory_backup.v_bkp_8u[binary_bytes] = dummy;
            binary_bytes++;
            if (binary_bytes == 1024)
                binary_full = 1;
        }
        else
        {
            if (prx1 < &rx1buff[SIZEOF_DATA - 1])
            {
                //al /r no le doy bola
                if (dummy == '\r')
                {
                }
                else if ((dummy == '\n') || (dummy == 26))		//26 es CTRL-Z
                {
                    *prx1 = '\0';
                    usart1_have_data = 1;
                }
                else
                {
                    *prx1 = dummy;
                    prx1++;
                }
            }
            else
                prx1 = rx1buff;    // solve blocking problem with garbage data
        }
    }

    // USART in Transmit Mode ----------------------------------------
    if (USART1->CR1 & USART_CR1_TXEIE)
    {
        if (USART1->ISR & USART_ISR_TXE)
        {
            if ((ptx1 < &tx1buff[SIZEOF_DATA]) && (ptx1 < ptx1_pckt_index))
            {
                USART1->TDR = *ptx1;
                ptx1++;
            }
            else
            {
                ptx1 = tx1buff;
                ptx1_pckt_index = tx1buff;
                USART1->CR1 &= ~USART_CR1_TXEIE;
            }
        }
    }

    if ((USART1->ISR & USART_ISR_ORE) || (USART1->ISR & USART_ISR_NE) || (USART1->ISR & USART_ISR_FE))
    {
        USART1->ICR |= 0x0e;
        dummy = USART1->RDR;
    }
}


void Usart1RxBinary (void)
{
    binary_bytes = 0;
    binary_full = 0;
}

void Usart1ChangeBaud (unsigned short new_baud)
{
    USART1->CR1 &= ~USART_CR1_UE;
    USART1->BRR = new_baud;
    USART1->CR1 |= USART_CR1_UE;
}

#endif    // USART1_IN_USE


//////////////////////
// Usart2 Functions //
//////////////////////
#ifdef USART2_IN_USE
void Usart2Config (void)
{
    unsigned int temp;
        
    if (!USART2_CLK)
        USART2_CLK_ON;

    // lo paso a 115200 por errores del esp
    // USART2->BRR = USART_115200;
    USART2->BRR = USART_9600;
    USART2->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;

    temp = GPIOA->AFR[0];
    temp &= 0xFFFF00FF;
    temp |= 0x00001100;	//PA3 -> AF1 PA2 -> AF1
    GPIOA->AFR[0] = temp;

    ptx2 = tx2buff;
    ptx2_pckt_index = tx2buff;
    prx2 = rx2buff;

    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, 7);
}


unsigned char Usart2ReadBuffer (unsigned char * bout, unsigned short max_len)
{
    unsigned int len;

    len = prx2 - rx2buff;

    if (len < max_len)
    {
        //prx2 points to the '\0' end of line                
        len += 1;
        memcpy(bout, (unsigned char *) rx2buff, len);
    }
    else
    {
        memcpy(bout, (unsigned char *) rx2buff, len);
        len = max_len;
    }

    //fix pointers positions
    prx2 = rx2buff;
    return (unsigned char) len;
}


void Usart2Send (char * send)
{
    unsigned char i;

    i = strlen(send);
    Usart2SendUnsigned((unsigned char *)send, i);
}


void Usart2SendUnsigned(unsigned char * send, unsigned char size)
{
    if ((ptx2_pckt_index + size) < &tx2buff[SIZEOF_DATA])
    {
        memcpy((unsigned char *)ptx2_pckt_index, send, size);
        ptx2_pckt_index += size;
        USART2->CR1 |= USART_CR1_TXEIE;
    }
}


void USART2_IRQHandler(void)
{
    unsigned char dummy;

    // USART in Receiv Mode ------------------------------------------
    if (USART2->ISR & USART_ISR_RXNE)
    {
        dummy = USART2->RDR & 0x0FF;

        if (prx2 < &rx2buff[SIZEOF_DATA])
        {
            if ((dummy == '\n') || (dummy == '\r') || (dummy == 26))		//26 es CTRL-Z
            {
                *prx2 = '\0';
                usart2_have_data = 1;
            }
            else
            {
                *prx2 = dummy;
                prx2++;
            }
        }
        else
            prx2 = rx2buff;    //soluciona problema bloqueo con garbage

    }
    
    // USART in Transmit Mode ----------------------------------------
    if (USART2->CR1 & USART_CR1_TXEIE)
    {
        if (USART2->ISR & USART_ISR_TXE)
        {
            if ((ptx2 < &tx2buff[SIZEOF_DATA]) && (ptx2 < ptx2_pckt_index))
            {
                USART2->TDR = *ptx2;
                ptx2++;
            }
            else
            {
                ptx2 = tx2buff;
                ptx2_pckt_index = tx2buff;
                USART2->CR1 &= ~USART_CR1_TXEIE;
            }
        }
    }

    if ((USART2->ISR & USART_ISR_ORE) || (USART2->ISR & USART_ISR_NE) || (USART2->ISR & USART_ISR_FE))
    {
        USART2->ICR |= 0x0e;
        dummy = USART2->RDR;
    }
}

#endif    // USART2_IN_USE

//--- end of file ---//
