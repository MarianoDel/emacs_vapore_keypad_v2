//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### KEYPAD.C #######################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "keypad.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "spi.h"
#include "display_7seg.h"
#include "parameters.h"
#include "rws317.h"
#include "siren_and_ampli.h"


// Externals -------------------------------------------------------------------
extern parameters_typedef param_struct;
extern volatile unsigned short code0;
extern volatile unsigned short code1;
extern unsigned char unlock_by_remote;



// Externals functions??? ------------------------------------------------------
extern void BuzzerCommands(unsigned char, unsigned char);


// Global ----------------------------------------------------------------------
unsigned char last_keypad_key = 0;
unsigned char keypad_state = 0;
unsigned char remote_keypad_state = 0;

volatile unsigned char keypad_timeout = 0;
volatile unsigned short keypad_interdigit_timeout = 0;
volatile unsigned short interdigit_timeout = 0;


// Private Module Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------

//se la llama para conocer la operacion de keypad seleccionada
//devuelve keypad_state KNUMBER_FINISH
//devuelve punteros a cada tecla en orden o posicion decimal con las 3 teclas combinadas
unsigned char CheckKeypad (unsigned char * sp0, unsigned char * sp1, unsigned char * sp2, unsigned short * posi)
{
    unsigned char switches = 0;

    switches = UpdateSwitches();

    switch (keypad_state)
    {
    case KNONE:

        if ((switches != NO_KEY) && (switches != STAR_KEY) && (switches != POUND_KEY))
        {
            //se presiono un numero voy a modo grabar codigo
            //reviso si fue 0
            if (switches == ZERO_KEY)
            {
                *sp0 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp0 = switches;
                Display_ShowNumbers(switches);
            }
            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            *sp1 = 0;
            *sp2 = 0;
            keypad_interdigit_timeout = param_struct.interdigit;
            keypad_state = KRECEIVING_A;
        }

        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;
        
        break;

    case KRECEIVING_A:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_B;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_B:			//segundo digito o confirmacion del primero
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;            

        if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
        {
            if (switches == ZERO_KEY)
            {
                *sp1 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp1 = switches;
                Display_ShowNumbers(switches);
            }
            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            keypad_state = KRECEIVING_C;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        //si esta apurado un solo numero
        if (switches == POUND_KEY)
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *sp2 = *sp0;
            *sp0 = 0;
            *posi = *sp2;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_C:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_D;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_D:				//tercer digito o confirmacion del segundo
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;

        if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
        {
            if (switches == ZERO_KEY)
            {
                *sp2 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp2 = switches;
                Display_ShowNumbers(switches);
            }

            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            keypad_state = KRECEIVING_E;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        //si esta apurado dos numeros
        if (switches == POUND_KEY)
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *sp2 = *sp0;
            *posi = *sp2 * 10 + *sp1;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_E:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_F;
            keypad_interdigit_timeout = param_struct.interdigit;

        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_F:
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;

        if (switches == POUND_KEY)	//es la confirmacion
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *posi = *sp0 * 100 + *sp1 * 10 + *sp2;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;
        break;

    case KCANCELLING:
            //se cancelo la operacion
            Display_ShowNumbers(DISPLAY_NONE);
            BuzzerCommands(BUZZER_HALF_CMD, 1);
            keypad_state = KCANCEL;
            
        break;
        
    case KNUMBER_FINISH:
    case KCANCEL:
    case KTIMEOUT:
    default:
        keypad_state = KNONE;
        break;
    }

    return keypad_state;
}


unsigned char CheckRemoteKeypad (unsigned char * sp0, unsigned char * sp1, unsigned char * sp2, unsigned short * posi)
{
    unsigned char button_remote = 0;

    switch (remote_keypad_state)
    {
    case RK_NONE:
        //me quedo esperando un código de control
        if (RxCode() == ENDED_OK)
        {
            //reviso aca si es de remote keypad o control
            //si es control contesto MUST_BE_CONTROL

            //en code0 y code1 tengo lo recibido
            button_remote = CheckButtonRemote(code0, code1);

            if (button_remote != REM_NO)
            {
                //se cancelo la operacion
                if (button_remote == REM_B10)
                {
                    Display_ShowNumbers(DISPLAY_NONE);
                    if (unlock_by_remote)
                        SirenCommands(SIREN_HALF_CMD);
                    BuzzerCommands(BUZZER_HALF_CMD, 1);
                    remote_keypad_state = RK_CANCEL;
                }
                else if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))	//es un numero 1 a 9 o 0
                {
                    //se presiono un numero - reviso si fue 0
                    if (button_remote == REM_B11)
                    {
                        Display_ShowNumbers(DISPLAY_ZERO);
                        *sp0 = 0;
                    }
                    else
                    {
                        Display_ShowNumbers(button_remote);
                        *sp0 = button_remote;
                    }
                    if (unlock_by_remote)
                        SirenCommands(SIREN_SHORT_CMD);
                    BuzzerCommands(BUZZER_SHORT_CMD, 1);

                    *sp1 = 0;
                    *sp2 = 0;
                    remote_keypad_state = RK_RECEIVING_A;
                    interdigit_timeout = 1000;
                }
            }
            else
            {
                remote_keypad_state = RK_MUST_BE_CONTROL;
            }
        }
        break;

    case RK_RECEIVING_A:
        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_RECEIVING_B;
            interdigit_timeout = param_struct.interdigit;
        }
        break;

    case RK_RECEIVING_B:			//segundo digito o confirmacion del primero
        if (RxCode() == ENDED_OK)
        {
            //en code0 y code1 tengo lo recibido
            button_remote = CheckButtonRemote(code0, code1);

            //se cancelo la operacion
            if (button_remote == REM_B10)
            {
                Display_ShowNumbers(DISPLAY_NONE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_HALF_CMD);
                BuzzerCommands(BUZZER_HALF_CMD, 1);
                remote_keypad_state = RK_CANCEL;
            }

            //si esta apurado un solo numero
            if (button_remote == REM_B12)
            {
                Display_ShowNumbers(DISPLAY_LINE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_SHORT_CMD);
                BuzzerCommands(BUZZER_SHORT_CMD, 2);
                *sp2 = *sp0;
                *sp1 = 0;
                *sp0 = 0;

                *posi = *sp2;
                remote_keypad_state = RK_NUMBER_FINISH;
            }
            //es un numero 1 a 9 o 0
            if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))
            {
                if (button_remote == REM_B11)
                {
                    *sp1 = 0;
                    Display_ShowNumbers(DISPLAY_ZERO);
                }
                else
                {
                    *sp1 = button_remote;
                    Display_ShowNumbers(button_remote);
                }

                if (unlock_by_remote)
                    SirenCommands(SIREN_SHORT_CMD);
                BuzzerCommands(BUZZER_SHORT_CMD, 1);
                *sp2 = 0;
                remote_keypad_state = RK_RECEIVING_C;
                interdigit_timeout = 1000;
            }
        }

        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_TIMEOUT;
        }

        break;

    case RK_RECEIVING_C:
        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_RECEIVING_D;
            interdigit_timeout = param_struct.interdigit;
        }
        break;

    case RK_RECEIVING_D:				//tercer digito o confirmacion del segundo
        if (RxCode() == ENDED_OK)
        {
            //en code0 y code1 tengo lo recibido
            button_remote = CheckButtonRemote(code0, code1);

            //se cancelo la operacion
            if (button_remote == REM_B10)
            {
                Display_ShowNumbers(DISPLAY_NONE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_HALF_CMD);
                BuzzerCommands(BUZZER_HALF_CMD, 1);
                remote_keypad_state = RK_CANCEL;
            }

            //si esta apurado dos numeros
            if (button_remote == REM_B12)
            {
                Display_ShowNumbers(DISPLAY_LINE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_SHORT_CMD);
                BuzzerCommands(BUZZER_SHORT_CMD, 2);
                *sp2 = *sp0;
                *sp0 = 0;

                *posi = *sp2 * 10 + *sp1;
                remote_keypad_state = RK_NUMBER_FINISH;
            }

            if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))	//es un numero 1 a 9 o 0
            {
                if (button_remote == ZERO_KEY)
                {
                    *sp2 = 0;
                    Display_ShowNumbers(DISPLAY_ZERO);
                }
                else
                {
                    *sp2 = button_remote;
                    Display_ShowNumbers(button_remote);
                }
                if (unlock_by_remote)
                    SirenCommands(SIREN_SHORT_CMD);
                BuzzerCommands(BUZZER_SHORT_CMD, 1);
                remote_keypad_state = RK_RECEIVING_E;
                interdigit_timeout = 1000;
            }
        }

        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_TIMEOUT;
        }
        break;

    case RK_RECEIVING_E:
        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_RECEIVING_F;
            interdigit_timeout = param_struct.interdigit;
        }
        break;

    case RK_RECEIVING_F:
        if (RxCode() == ENDED_OK)
        {
            //en code0 y code1 tengo lo recibido
            button_remote = CheckButtonRemote(code0, code1);

            //se cancelo la operacion
            if (button_remote == REM_B10)
            {
                Display_ShowNumbers(DISPLAY_NONE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_HALF_CMD);
                BuzzerCommands(BUZZER_HALF_CMD, 1);
                remote_keypad_state = RK_CANCEL;
            }

            //es la confirmacion
            if (button_remote == REM_B12)
            {
                Display_ShowNumbers(DISPLAY_LINE);
                if (unlock_by_remote)
                    SirenCommands(SIREN_SHORT_CMD);
                BuzzerCommands(BUZZER_SHORT_CMD, 2);
                *posi = *sp0 * 100 + *sp1 * 10 + *sp2;
                remote_keypad_state = RK_NUMBER_FINISH;
            }
        }

        if (!interdigit_timeout)
        {
            remote_keypad_state = RK_TIMEOUT;
        }
        break;

    case RK_MUST_BE_CONTROL:
    case RK_CANCEL:
    case RK_TIMEOUT:
    default:
        remote_keypad_state = RK_NONE;
        break;
    }
    return remote_keypad_state;
}


void Keypad_Timeouts (void)
{
    if (keypad_timeout)
        keypad_timeout--;

    if (keypad_interdigit_timeout)
        keypad_interdigit_timeout--;

    if (interdigit_timeout)
        interdigit_timeout--;
        
}
//--- end of file ---//
