//----------------------------------------------------------------------
// #### PROYECTO DEXEL/LUIS_CASINO 6CH BIDIRECCIONAL - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### HARD.C #########################################################
//----------------------------------------------------------------------
#include "hard.h"
#include "stm32f0xx.h"
// #include "parameters.h"

#include <string.h>


// Module Private Types Constants and Macros -----------------------------------
#define TT_BUZZER_BIP_SHORT		50
#define TT_BUZZER_BIP_SHORT_WAIT	100
#define TT_BUZZER_BIP_HALF		200
#define TT_BUZZER_BIP_HALF_WAIT		500
#define TT_BUZZER_BIP_LONG		1200
#define TT_BUZZER_BIP_LONG_WAIT		1500


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
volatile unsigned char switches_timer = 0;
volatile unsigned short buzzer_timeout = 0;
volatile unsigned short display_timeout = 0;
unsigned char last_sw = 0;


// Module Private Functions ----------------------------------------------------



// Module Functions ------------------------------------------------------------
void HARD_Timeouts (void)
{
    if (switches_timer)
        switches_timer--;

    if (buzzer_timeout)
        buzzer_timeout--;

    if (display_timeout)
        display_timeout--;
    
    // HARD_UpdateSwitches();    //call it on main or subroutines

}



// Module Functions ------------------------------------------------------------
unsigned char HARD_UpdateSwitches (void)
{
    if (switches_timer)
        return last_sw;

    unsigned char sw = 0;    // NO_KEY
    switches_timer = 50;           

    if (KEY_1)
        sw = 1;
    else if (KEY_2)
        sw = 2;
    else if (KEY_3)
        sw = 3;
    else if (KEY_4)
        sw = 4;
    else if (KEY_5)
        sw = 5;
    else if (KEY_6)
        sw = 6;
    else if (KEY_7)
        sw = 7;
    else if (KEY_8)
        sw = 8;
    else if (KEY_9)
        sw = 9;
    else if (KEY_STAR)
        sw = 10;
    else if (KEY_0)
        sw = 11;
    else if (KEY_POUND)
        sw = 12;

    last_sw = sw;
    return sw;
}


unsigned char HARD_GetSwitches (void)
{
    //revisa los switches cada 10ms
    // if (!switches_timer)
    // {
    //     if (S1_PIN)
    //         s1++;
    //     else if (s1 > 50)
    //         s1 -= 50;
    //     else if (s1 > 10)
    //         s1 -= 5;
    //     else if (s1)
    //         s1--;

    //     if (S2_PIN)
    //         s2++;
    //     else if (s2 > 50)
    //         s2 -= 50;
    //     else if (s2 > 10)
    //         s2 -= 5;
    //     else if (s2)
    //         s2--;

    //     if (S3_PIN)
    //         s3++;
    //     else if (s3 > 50)
    //         s3 -= 50;
    //     else if (s3 > 10)
    //         s3 -= 5;
    //     else if (s3)
    //         s3--;

    //     if (S4_PIN)
    //         s4++;
    //     else if (s4 > 50)
    //         s4 -= 50;
    //     else if (s4 > 10)
    //         s4 -= 5;
    //     else if (s4)
    //         s4--;
        
    //     switches_timer = SWITCHES_TIMER_RELOAD;
    // }
    return 0;
}


// Module Functions ------------------------------------------------------------
// Buzzer Bips States
typedef enum
{    
    BUZZER_WAIT_COMMANDS = 0,
    BUZZER_MARK,
    BUZZER_SPACE,
    BUZZER_TO_STOP
    
} buzzer_state_t;

buzzer_state_t buzzer_state = BUZZER_WAIT_COMMANDS;
unsigned char buzzer_multiple = 0;
unsigned short buzzer_timer_reload_mark = 0;
unsigned short buzzer_timer_reload_space = 0;

void BuzzerCommands(unsigned char command, unsigned char multiple)
{
    if (command == BUZZER_STOP_CMD)
        buzzer_state = BUZZER_TO_STOP;
    else if (command == BUZZER_LONG_CMD)
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_LONG;
        buzzer_timer_reload_space = TT_BUZZER_BIP_LONG_WAIT;
    }
    else if (command == BUZZER_HALF_CMD)
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_HALF;
        buzzer_timer_reload_space = TT_BUZZER_BIP_HALF_WAIT;
    }
    else
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_SHORT;
        buzzer_timer_reload_space = TT_BUZZER_BIP_SHORT_WAIT;
    }

    buzzer_state = BUZZER_MARK;
    buzzer_timeout = 0;
    buzzer_multiple = multiple;
}


void BuzzerUpdate (void)
{
    switch (buzzer_state)
    {
        case BUZZER_WAIT_COMMANDS:
            break;

        case BUZZER_MARK:
            if (!buzzer_timeout)
            {
                BUZZER_ON;
                buzzer_state++;
                buzzer_timeout = buzzer_timer_reload_mark;
            }
            break;

        case BUZZER_SPACE:
            if (!buzzer_timeout)
            {
                if (buzzer_multiple > 1)
                {
                    buzzer_multiple--;
                    BUZZER_OFF;
                    buzzer_state = BUZZER_MARK;
                    buzzer_timeout = buzzer_timer_reload_space;
                }
                else
                    buzzer_state = BUZZER_TO_STOP;
            }
            break;
            
        case BUZZER_TO_STOP:
        default:
            BUZZER_OFF;
            buzzer_state = BUZZER_WAIT_COMMANDS;
            break;
    }
}


unsigned char BuzzerIsFree (void)
{
    if (buzzer_state == BUZZER_WAIT_COMMANDS)
        return 1;

    return 0;
}


unsigned char Led_Is_On (void)
{
    return LED;
}


void Led_On (void)
{
    LED_ON;
}


void Led_Off (void)
{
    LED_OFF;
}


void Charge_On (void)
{
    CHARGE_ON;
}


void Charge_Off (void)
{
    CHARGE_OFF;
}

//--- end of file ---//
