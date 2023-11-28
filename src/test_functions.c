//----------------------------------------------------------
// ## Internal Test Functions Module
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEST_FUNCTIONS.C ###################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "test_functions.h"
#include "hard.h"
#include "adc.h"
#include "usart.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "spi.h"

#include "display_7seg.h"
#include "sst25.h"
#include "sst25codes.h"
#include "siren_and_ampli.h"

#include "memory_utils.h"


#include <stdio.h>
// Externals -------------------------------------------------------------------
extern volatile unsigned short adc_ch [];
extern volatile unsigned short timer_standby;


// Globals ---------------------------------------------------------------------


// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------
void TF_Led (void);
void TF_Display7Seg (void);
void TF_Usart1_Tx_Single (void);
void TF_Switches_And_Display7Seg (void);
void TF_Switches_And_Display7Seg_Buzzer (void);
void TF_Display7Seg_Charge (void);
void TF_Display7Seg_FPLUS_F12PLUS (void);
void TF_Usart1_TxRx (void);
void TF_Voltage_Temperature (void);
void TF_RIGHT_LEFT (void);
void TF_TIM1_HBridge (void);
void TF_Usart1_And_Memory_Only_Jedec (void);
void TF_Usart1_And_Memory_RW (void);
void TF_Usart1_And_Memory_Debug (void);
void TF_TIM1_HBridge_Siren (void);
void TF_TIM1_HBridge_Audio (void);



// Module Functions ------------------------------------------------------------
void TF_Hardware_Tests (void)
{
    // TF_Led ();
    // TF_Display7Seg ();
    // TF_Switches_And_Display7Seg ();
    // TF_Switches_And_Display7Seg_Buzzer ();
    // TF_Display7Seg_Charge ();
    // TF_Display7Seg_FPLUS_F12PLUS ();
    // TF_Usart1_TxRx ();
    // TF_Voltage_Temperature ();
    // TF_RIGHT_LEFT ();
    // TF_TIM1_HBridge ();
    // TF_Usart1_And_Memory_Only_Jedec ();
    // TF_Usart1_And_Memory_RW ();
    TF_Usart1_And_Memory_Debug ();
    // TF_TIM1_HBridge_Siren ();
    // TF_TIM1_HBridge_Audio ();
    
}


void TF_Led (void)
{
    while (1)
    {
        if (LED)
            LED_OFF;
        else
            LED_ON;

        Wait_ms(200);
    }
}


void TF_Display7Seg (void)
{
    while (1)
    {        
        for (int i = 0; i < SIZEOF_VDISPLAY; i++)
        {
            Display_ShowNumbers(i);
            Wait_ms(800);
            Display_ShowNumbers(DISPLAY_NONE);
            Wait_ms(200);
        }

        Wait_ms (3000);
    }
}


#define NO_KEY 0
#define NINE_KEY 9
#define STAR_KEY 10
#define ZERO_KEY 11
#define POUND_KEY 12
void TF_Switches_And_Display7Seg (void)
{
    unsigned char switches = 0;
    unsigned char last_switches = 0;
    
    while (1)
    {
        Wait_ms (30);
        
        switches = HARD_UpdateSwitches();

        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            last_switches = switches;
        }
        else
        {
            if (last_switches != switches)
            {
                last_switches = switches;
                if (switches == ZERO_KEY)
                    Display_ShowNumbers(DISPLAY_ZERO);
                else if (switches == STAR_KEY)
                    Display_ShowNumbers(DISPLAY_SQR_UP);
                else if (switches == POUND_KEY)
                    Display_ShowNumbers(DISPLAY_SQR_DOWN);
                else
                {
                    Display_ShowNumbers(switches);
                }
            }
        }
    }
}


void TF_Switches_And_Display7Seg_Buzzer (void)
{
    unsigned char switches = 0;
    unsigned char last_switches = 0;
    
    while (1)
    {
        if (!timer_standby)
        {
            timer_standby = 30;
            
            switches = HARD_UpdateSwitches();

            if (switches == NO_KEY)
            {
                Display_ShowNumbers(DISPLAY_NONE);
                last_switches = switches;
            }
            else
            {
                if (last_switches != switches)
                {
                    last_switches = switches;
                    if (switches == ZERO_KEY)
                        Display_ShowNumbers(DISPLAY_ZERO);
                    else if (switches == STAR_KEY)
                        Display_ShowNumbers(DISPLAY_SQR_UP);
                    else if (switches == POUND_KEY)
                        Display_ShowNumbers(DISPLAY_SQR_DOWN);
                    else
                    {
                        Display_ShowNumbers(switches);
                    }

                    BuzzerCommands(BUZZER_SHORT_CMD, 1);
                }
            }
        }

        BuzzerUpdate ();
    }
}


void TF_Display7Seg_Charge (void)
{
    while (1)
    {
        CHARGE_ON;
        Display_ShowNumbers(1);
        Wait_ms(5000);

        CHARGE_OFF;
        Display_ShowNumbers(0);
        Wait_ms(5000);

    }
}


void TF_Display7Seg_FPLUS_F12PLUS (void)
{
    while (1)
    {
        FPLUS_ON;
        F12PLUS_OFF;
        Display_ShowNumbers(1);
        Wait_ms(5000);

        FPLUS_OFF;
        F12PLUS_ON;
        Display_ShowNumbers(2);
        Wait_ms(5000);

    }
}


void TF_Usart1_TxRx (void)
{
    for (unsigned char i = 0; i < 5; i++)
    {
        LED_ON;
        Wait_ms(250);
        LED_OFF;
        Wait_ms(250);
    }

    Display_ShowNumbers(0);
    Usart1Config();

    char s_to_send [100] = { 0 };
    Usart1Send("Ready to test...\n");
    while (1)
    {
        if (Usart1HaveData ())
        {
            Usart1HaveDataReset ();
            
            if (LED)
                LED_OFF;
            else
                LED_ON;
            
            Usart1ReadBuffer((unsigned char *) s_to_send, 100);
            Wait_ms(1000);
            Usart1Send("-> ");
            Usart1Send(s_to_send);
            Usart1Send("\r\n");
        }
    }
}


void TF_Voltage_Temperature (void)
{
    char s_to_send [100];
    unsigned short seq_cnt = 0;
    float volts = 0.0;
    unsigned char volts_int = 0;
    unsigned char volts_dec = 0;

    // Adc and Dma Init
    AdcConfig();

    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    ADC1->CR |= ADC_CR_ADSTART;
    
    // Usart Init
    Usart1Config();
    Usart1Send("Measuring Data..\n");
    
    while (1)
    {
        if (sequence_ready)
        {
            sequence_ready_reset;
            seq_cnt++;
            if (seq_cnt > (37037 - 1))    // 3 samples at (12.5 + 239.5) . 1/14MHz = 54us
            {
                seq_cnt = 0;
                volts = Sense_IN * 3.3 * 5.54;    // scale (18.3 / 3.3)
                volts = volts / 4095;
                volts_int = (unsigned char) volts;
                volts = volts - volts_int;
                volts = volts * 100;
                volts_dec = (unsigned char) volts;
                sprintf(s_to_send, "ntc: %d in: %d bat: %d voltage sense in: %d.%02dV\n",
                        Sense_NTC,
                        Sense_IN,
                        Sense_BAT,                        
                        volts_int,
                        volts_dec);
                Usart1Send(s_to_send);
            }
        }
    }
}


void TF_RIGHT_LEFT (void)
{
    Gpio_PA7_PA8_Output ();
    
    while (1)
    {
        LEFT_OFF;
        RIGHT_ON;
        Wait_ms(3000);
        
        RIGHT_OFF;
        LEFT_ON;
        Wait_ms(3000);
        
    }
}


void TF_TIM1_HBridge (void)
{
    TIM1_Init ();

    // Update_TIM1_CH1 (DUTY_50_FREQ_1000HZ);
    Update_TIM1_CH1 (500);

    while (1);
}


void TF_Usart1_And_Memory_Only_Jedec (void)
{
    unsigned char buffer [3];
    char my_str [100];

    // SPI Init
    CE_OFF;
    WP_OFF;    
    SPI_Config();
    
    // Usart Init
    Usart1Config();
    Usart1Send("Reading Memory...\n");

    while (1)
    {
        Usart1Send("memory jedec: ");
        getJEDEC (buffer);
        sprintf(my_str, "0x%02x 0x%02x 0x%02x\n",
                buffer[0],
                buffer[1],
                buffer[2]);

        Usart1Send(my_str);
        
        Wait_ms (2000);
        
    }
}


void TF_Usart1_And_Memory_RW (void)
{
    unsigned char buffer [3];
    char my_str [100];

    // SPI Init
    CE_OFF;
    WP_OFF;
    SPI_Config();
    
    // Usart Init
    Usart1Config();
    Usart1Send("Reading Memory...\n");

    while (1)
    {
        Usart1Send("memory jedec: ");
        getJEDEC (buffer);
        sprintf(my_str, "0x%02x 0x%02x 0x%02x\n",
                buffer[0],
                buffer[1],
                buffer[2]);

        Usart1Send(my_str);
        
        Wait_ms (1000);

        for (int i = 888; i < 1000; i++)
        {
            sprintf(my_str, "position %d saving: %d", i, 0x50);
            Usart1Send(my_str);
            if (SST_WriteCodeToMemory(i, 0x50) != 0)
                Usart1Send(" ok");
            else
                Usart1Send(" err");
            
            Wait_ms(100);

            int get = SST_CheckIndexInMemory(i);
            sprintf(my_str, " getting: %d\n", get);
            Usart1Send(my_str);

            Wait_ms(1000);
        }
    }
}


extern void writeSPI2 (unsigned char data);
extern void writeStatus2NVM (unsigned char data);
extern void unprotectNVM (void);
void TF_Usart1_And_Memory_Debug (void)
{
    unsigned char buffer [3];
    char my_str [100];

    // SPI Init
    CE_OFF;
    WP_OFF;
    SPI_Config();
    
    // Usart Init
    Usart1Config();
    Wait_ms(20);
    Usart1Send("\n\nmemory jedec: ");
    getJEDEC (buffer);
    sprintf(my_str, "0x%02x 0x%02x 0x%02x\n",
            buffer[0],
            buffer[1],
            buffer[2]);

    // read first jedec to initialize memory
    Usart1Send(my_str);
    Wait_ms(200);
    unsigned char send_menu = 1;
    unsigned char data = 0;
    unsigned char data1 = 0; 
    unsigned char data2 = 0;
    unsigned int pos_to_test = 0x3200;
    
    while (1)
    {
        if (send_menu)
        {
            Usart1Send("  read status register 1to3 RDSR: 1\n");
            Usart1Send("  enable write: 2\n");
            Usart1Send("  clear all mem: 3\n");
            Wait_ms(100);
            sprintf(my_str, "  blank 4k from 0x%04x: 4\n", pos_to_test);
            Usart1Send(my_str);
            sprintf(my_str, "  read position 0x%04x: 5\n", pos_to_test);
            Usart1Send(my_str);
            sprintf(my_str, "  write position 0x%04x: 6\n", pos_to_test);            
            Usart1Send(my_str);
            Wait_ms(100);
            Usart1Send("  write SR2: 7\n");
            Usart1Send("  unprotect: 8\n");
            Usart1Send("\n   selection: ");
            send_menu = 0;
        }

        if (Usart1HaveData())
        {
            Usart1HaveDataReset();
            Usart1ReadBuffer((unsigned char *) my_str, sizeof(my_str));

            switch (my_str[0])
            {
            case '1':
                data = readStatusNVM();
                data1 = readStatus2NVM();
                data2 = readStatus3NVM();
                sprintf(my_str, "status 0x%02x 0x%02x 0x%02x\n", data, data1, data2);
                Usart1Send(my_str);
                Wait_ms(500);                
                break;

            case '2':
                NVM_On;
                writeSPI2(SST25_WREN);
                NVM_Off;
                Wait_ms(500);                
                break;

            case '3':
                clearNVM();
                Usart1Send("done!\n");
                Wait_ms(500);                
                break;

            case '4':
                Clear4KNVM(pos_to_test);
                Usart1Send("done!\n");
                Wait_ms(500);                
                break;
                
            case '5':
                readBufNVM8u(&data, 1, pos_to_test);
                sprintf(my_str, "0x%02x\n", data);
                Usart1Send(my_str);
                Wait_ms(500);
                break;

            case '6':
                writeNVM(0x55, pos_to_test);
                sprintf(my_str, "saving 0x55 on 0x%04x\n", pos_to_test);
                Usart1Send(my_str);
                Wait_ms(500);                
                break;

            case '7':
                writeStatus2NVM(0x00);
                Usart1Send("sr2 to 0x00\n");
                Wait_ms(500);                
                break;

            case '8':
                unprotectNVM();
                Usart1Send("memory unprotected\n");
                Wait_ms(500);                
                break;
                
            case '?':
                send_menu = 1;
                break;

            default:
                Usart1Send("Selection error! try ?\n");
                break;
            }
        }
    }
}


void TF_TIM1_HBridge_Siren (void)
{
    unsigned short ms_cnt = 0;
    unsigned char siren_on = 0;
    
    TIM1_Init ();

    Ampli_to_Sirena();
    
    while (1)
    {
        if (siren_on)
        {
            if (!ms_cnt)
            {
                siren_on = 0;
                ms_cnt = 2000;
                Power_Ampli_Disa ();
            }
        }
        else
        {
            if (!ms_cnt)
            {
                siren_on = 1;
                ms_cnt = 2000;
                SirenCommands(SIREN_MULTIPLE_UP_CMD);
            }                
        }

        UpdateSiren ();
        
        if (!timer_standby)
        {
            timer_standby = 1;
            Siren_Timeouts ();
            if (ms_cnt)
                ms_cnt--;
        }
    }
}


void TF_TIM1_HBridge_Audio (void)
{
    unsigned short ms_cnt = 0;
    unsigned short i = 0;

    // SPI and Memory init
    CE_OFF;
    WP_OFF;    
    SPI_Config();
    
    // Usart Init
    Usart1Config();
    Usart1Send("Reading Memory...\n");

    TIM1_Init ();
    
    Ampli_to_Sirena ();
        
    AudioInit();

    ShowFileSystem();
    
    while (1)
    {

        UpdateAudio ();

        if (!ms_cnt)
        {
            ms_cnt = 10000;
            PositionToSpeak (i);

            if (i < 1000)
                i++;
            else
                i = 0;
        }
            
        if (!timer_standby)
        {
            timer_standby = 1;
            if (ms_cnt)
                ms_cnt--;
        }
        
    }
}


//--- end of file ---//
