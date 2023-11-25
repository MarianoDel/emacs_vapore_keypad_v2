//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### BATTERY_AND_CHARGER.C ##############################
//----------------------------------------------------------
#include "battery_and_charger.h"
#include "hard.h"
#include "usart.h"
#include "adc.h"
#include "dsp.h"

#include <stdio.h>
#include <string.h>


// Module Private Types Constants and Macros -----------------------------------
// Measurements for Sense_IN
// scale -> 18.3 / 4095
// NTC 1k5 90deg 3k 70deg

#define VAC_GOOD    1902    // 8.5V
#define VBATT_FOR_LOW    2013    //9V
#define VBATT_FOR_GOOD    2461    //11V
#define VNTC_FOR_CUT    534    //0.43V on NTC 1k5@90deg
#define VNTC_FOR_CHARGE    945    //0.761V on NTC 3k@70deg


typedef enum {
    BATTERY_CHECKS_INIT,
    AC_PRESENT_BATTERY_CHARGING,
    AC_NOT_BATTERY_GOOD,
    AC_NOT_BATTERY_LOW    
    
} battery_state_e;




// Externals -------------------------------------------------------------------
extern volatile unsigned short adc_ch [];


// Globals ---------------------------------------------------------------------
volatile unsigned short battery_led_timer = 0;
volatile unsigned char charger_meas_timer = 0;
unsigned short battery_led_period = 0;
battery_state_e battery_state = BATTERY_CHECKS_INIT;

unsigned char ntc_disconneted_for_siren = 0;
ma16_u16_data_obj_t vac_filter;
ma16_u16_data_obj_t vbatt_filter;
ma16_u16_data_obj_t vntc_filter;

unsigned short vac_filtered = 0;
unsigned short vbatt_filtered = 0;
unsigned short vntc_filtered = 0;


// Module Private Functions ----------------------------------------------------
unsigned char Battery_Check_AC_Is_Good (void);
void Charger_Update (void);
void Battery_Filters_Init(void);



// Module Functions ------------------------------------------------------------
void Battery_Update (void)
{
    switch (battery_state)
    {
    case BATTERY_CHECKS_INIT:
        Battery_Filters_Init();
        battery_led_period = 400;
        battery_state++;
        break;
        
    case AC_PRESENT_BATTERY_CHARGING:
        if (!Battery_Check_AC_Is_Good())
        {
            Usart1Send("\r\nKeypad on Batt\r\n");            
            battery_state = AC_NOT_BATTERY_GOOD;
            battery_led_period = 1200;
        }

        Charger_Update();
        
        break;

    case AC_NOT_BATTERY_GOOD:
        if (Battery_Check_AC_Is_Good())
        {            
            Usart1Send("\r\nKeypad with AC\r\n");
            battery_state = AC_PRESENT_BATTERY_CHARGING;
            battery_led_period = 400;
        }
        else if (vbatt_filtered < VBATT_FOR_LOW)
        {
            Usart1Send("\r\nKeypad Low Batt!!!\r\n");
            battery_state = AC_NOT_BATTERY_LOW;
        }
        break;

    case AC_NOT_BATTERY_LOW:
        if (Battery_Check_AC_Is_Good())
        {            
            Usart1Send("\r\nKeypad with AC\r\n");
            battery_state = AC_PRESENT_BATTERY_CHARGING;
            battery_led_period = 400;
        }
        else if (vbatt_filtered > VBATT_FOR_GOOD)
        {
            Usart1Send("\r\nKeypad on Batt\r\n");
            battery_state = AC_NOT_BATTERY_GOOD;
        }
        break;

    default:
        battery_state = AC_PRESENT_BATTERY_CHARGING;
        break;
        
    }

    // all cases functions

    // with AC toggle led on 400ms
    // without AC toggle led on 1200ms    
    if (!battery_led_timer)
    {
        battery_led_timer = battery_led_period;

        if (Led_Is_On())
            Led_Off();
        else
            Led_On();

    }

    if (!charger_meas_timer)
    {
        charger_meas_timer = 10;
        vac_filtered = MA16_U16Circular (&vac_filter, Sense_IN);
        vbatt_filtered = MA16_U16Circular (&vbatt_filter, Sense_BAT);
        vntc_filtered = MA16_U16Circular (&vntc_filter, Sense_NTC);
    }
}


void Battery_Filters_Init (void)
{
    MA16_U16Circular_Reset (&vac_filter);
    MA16_U16Circular_Reset (&vbatt_filter);    
    MA16_U16Circular_Reset (&vntc_filter);

    for (int i = 0; i < 16; i++)
    {
        vac_filtered = MA16_U16Circular (&vac_filter, Sense_IN);
        vbatt_filtered = MA16_U16Circular (&vbatt_filter, Sense_BAT);
        vntc_filtered = MA16_U16Circular (&vntc_filter, Sense_NTC);        
    }
}


unsigned char Battery_Check_AC_Is_Good (void)
{
    if (vac_filtered > VAC_GOOD)
        return 1;

    return 0;
}


void Charger_Update (void)
{
    if (!ntc_disconneted_for_siren)
    {
        if (vntc_filtered < VNTC_FOR_CUT)
            Charge_Off();
        else if (vntc_filtered > VNTC_FOR_CHARGE)
            Charge_On();
        
    }        
}


void Charger_Disconnect_For_Siren (void)
{
    ntc_disconneted_for_siren = 1;
    Charge_Off();
}


void Charger_Connect (void)
{
    ntc_disconneted_for_siren = 0;
}


void Battery_Timeouts (void)
{
    if (battery_led_timer)
        battery_led_timer--;

    if (charger_meas_timer)
        charger_meas_timer--;
}


//--- end of file ---//

