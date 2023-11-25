//----------------------------------------------------------
// #### PROYECTO DEXEL 6CH BIDIRECCIONAL - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MAIN.C #############################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "hard.h"
#include "stm32f0xx.h"
#include "gpio.h"
#include "usart.h"

// #include "core_cm0.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "spi.h"

#include "comms.h"
// #include "dsp.h"


// hardware tests functions
#include "test_functions.h"
#include "factory_test.h"
// #include "temperatures.h"    // for defines on lm335 and ntc

#include "flash_program.h"
#include "memory_conf.h"
#include "porton_kirno.h"
#include "gestion.h"
#include "production.h"
#include "keypad.h"
#include "rws317.h"

#include <stdio.h>
#include <string.h>


// Externals -------------------------------------------------------------------
parameters_typedef mem_conf;
mem_bkp_typedef memory_backup;
filesystem_typedef files;
parameters_typedef param_struct;

// for gestion module
volatile unsigned char binary_full = 0;
unsigned char next_pckt = 0;
unsigned char file_done = 0;


// --------- Externals del ADC ---------
volatile unsigned short adc_ch [ADC_CHANNEL_QUANTITY];
volatile unsigned char seq_ready;


// --------- Modules: porton_kirno -----------
volatile unsigned short siren_timeout;

// Globals ---------------------------------------------------------------------
// -- for the ms timers ----------------
volatile unsigned short timer_standby = 0;


// Module Private Functions ----------------------------------------------------
void TimingDelay_Decrement(void);
void SysTickError (void);


// Module Functions ------------------------------------------------------------
int main(void)
{
    // Gpio Configuration.
    GPIO_Config();

    // Systick Timer Activation
    if (SysTick_Config(48000))
        SysTickError();

    // Tests Functions
    // TF_Hardware_Tests ();


    ///////////////////////////////////
    // Send Hardware to a Know State //
    ///////////////////////////////////
    CE_OFF;
    WP_OFF;
    FPLUS_OFF;
// #ifdef USE_F12_PLUS_WITH_SM
//     F12_State_Machine_Reset();
// #else
    F12PLUS_OFF;
// #endif
    BUZZER_OFF;
    LED_OFF;
    
        
    /////////////////////////
    // Mains Programs Init //
    /////////////////////////    
#ifdef PROGRAM_FACTORY_TEST
    // Tests for the hardware onboard
    
    SPI_Config();
    Usart1Config();

    FuncFactoryTest();

#endif

    
#ifdef PROGRAMA_PORTON_KIRNO
    // Tests: move relay w/timer on good codes

    SPI_Config();
    Usart1Config();
    RxCode();    // clean rxcode init
    TIM3_Init();
    
    FuncPortonKirno();

#endif

    
#ifdef PROGRAMA_DE_GESTION
    // Saves configurations and Memory

    SPI_Config();
    Usart1Config();
    Usart1ChangeBaud(USART_115200);    
    
    FuncGestion();

#endif


#ifdef PROGRAMA_DE_PRODUCCION    

    SPI_Config();
    Usart1Config();
    RxCode();    // clean rxcode init
    TIM3_Init();

    TIM1_Init();
    Power_Ampli_Disa();

    Production_Function();
    
#endif


#ifdef PROGRAMA_DE_PRODUCCION_Y_GESTION
    
    // Saves configurations and Memory (only if starts presing #1)
    if (KEY_1)
    {
        // go to gestion program
        SPI_Config();
        Usart1Config();
        Usart1ChangeBaud(USART_115200);    
    
        FuncGestion();
    }
    
    SPI_Config();
    Usart1Config();
    RxCode();    // clean rxcode init
    TIM3_Init();

    TIM1_Init();
    Power_Ampli_Disa();

    Production_Function();
    
#endif
    
    return 0;
}
//--- End of Main ---//




void TimingDelay_Decrement(void)
{
    TIM_Timeouts ();

    HARD_Timeouts ();    
    
    if (timer_standby)
        timer_standby--;
    
#if defined (PROGRAMA_DE_PRODUCCION) ||\
    defined (PROGRAMA_DE_PRODUCCION_Y_GESTION)
    
    Production_Timeouts();

    Keypad_Timeouts();
    
#endif
    
}


void SysTickError (void)
{
    //Capture systick error...
    while (1)
    {
        if (CTRL_FAN)
            CTRL_FAN_OFF;
        else
            CTRL_FAN_ON;

        for (unsigned char i = 0; i < 255; i++)
        {
            asm ("nop \n\t"
                 "nop \n\t"
                 "nop \n\t" );
        }
    }
}


//--- end of file ---//

