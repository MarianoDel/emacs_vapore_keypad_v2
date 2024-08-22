//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PRODUCTION.C #######################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "production.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "usart.h"
#include "tim.h"

#include "display_7seg.h"
#include "memory_utils.h"
#include "memory_conf.h"
#include "parameters.h"
#include "keypad.h"
#include "sst25.h"
#include "sst25codes.h"
#include "siren_and_ampli.h"
#include "rws317.h"
#include "comms.h"
#include "battery_and_charger.h"
#include "gestion.h"

#include <stdio.h>
#include <string.h>

// Module Private Types Constants and Macros -----------------------------------


//ESTADOS DEL PROGRAMA PRINCIPAL
typedef enum {
    MAIN_INIT,
    MAIN_MAIN,	
    MAIN_IN_ALARM,	
    MAIN_TO_UNLOCK,
    MAIN_UNLOCK,
    MAIN_TO_DEL_CODE,
    MAIN_TO_SAVE_AT_LAST,
    MAIN_TO_CHANGE_USER_PASSWORD,
    MAIN_TO_MAIN_CANCEL,
    MAIN_TO_MAIN_OK,
    MAIN_TO_MAIN_TIMEOUT,
    MAIN_TO_MAIN_WAIT_5SEGS,
    MAIN_TO_MAIN_WAIT_5SEGSA,
    MAIN_TO_MASS_ERASE_AT_LAST,
    MAIN_TO_SAVE_IN_SEQUENCE,
    MAIN_TO_SAVE_IN_SEQUENCE_WAITING

} main_state_e;


typedef enum
{
    WORKING = 1,
    END_OK,
    END_ERROR,
    END_TIMEOUT
        
} alarm_result_e;

//ESTADOS DE LA ALARMA
#define ALARM_START				0
#define ALARM_BUTTON1			10
#define ALARM_BUTTON1_A			11
#define ALARM_BUTTON1_B			12
#define ALARM_BUTTON1_C			13
#define ALARM_BUTTON1_D			14
#define ALARM_BUTTON1_FINISH	15
#define ALARM_BUTTON1_FINISH_B	16

#define ALARM_NO_CODE			100

#define ALARM_BUTTON2		20
#define ALARM_BUTTON2_A		21
#define ALARM_BUTTON2_B		22
#define ALARM_BUTTON2_C		23
#define ALARM_BUTTON2_D			24
#define ALARM_BUTTON2_E			25
#define ALARM_BUTTON2_FINISH	26

#define ALARM_BUTTON3			30
#define ALARM_BUTTON3_A			31
#define ALARM_BUTTON3_B			32
#define ALARM_BUTTON3_C			33
#define ALARM_BUTTON3_D			34
#define ALARM_BUTTON3_FINISH	35
#define ALARM_BUTTON3_FINISH_B	36

#define ALARM_BUTTON4			40
#define ALARM_BUTTON4_A			41
#define ALARM_BUTTON4_FINISH	42


// Externals -------------------------------------------------------------------
extern mem_bkp_typedef memory_backup;
extern filesystem_typedef files;
extern parameters_typedef param_struct;


// Globals ---------------------------------------------------------------------
volatile unsigned short timer_keypad_enabled = 0;
volatile unsigned short wait_for_code_timeout;
volatile unsigned short interdigit_timeout;

// volatile unsigned short siren_timeout;

// volatile unsigned short f12_plus_timer;
volatile unsigned char button_timer_secs = 0;
volatile unsigned short button_timer_internal = 0;
volatile unsigned short code0 = 0;
volatile unsigned short code1 = 0;


//--- Respecto de las alarmas y botones
unsigned short last_one_or_three = 0;
unsigned short last_two = 0;
unsigned char alarm_state = 0;
unsigned char repetition_counter = 0;

#ifdef USE_BUTTON4_ONLY_5MINS
#define	BUTTON4_5MINS_TT    (1000 * 60 * 5)
volatile unsigned int button4_only5mins_timeout = 0;
#endif

//--- Respecto del KeyPad
// unsigned char remote_keypad_state = 0;
unsigned char unlock_by_remote = 0;

volatile unsigned char pilot_code = 0;
volatile unsigned char errorcode = 0;


// Private Module Functions ----------------------------------------------------
unsigned char FuncAlarm (unsigned char sms_alarm);
unsigned char Production_Check_Monitoring (void);

#ifdef PRODUCCION_CHICKEN_BUCLE
unsigned char FuncAlarmChickenBucle (unsigned char sms_alarm);
#endif


// Module Functions ------------------------------------------------------------
void Production_Function (void)
{
    Usart1Send((char *) " - Kirno Technology - STM32F030\r\n");
    Wait_ms(100);
    Usart1Send((char *) "Sistema de Alarma ALERTA VECINAL\r\n");
    Wait_ms(100);
    
#ifdef HARD
    Usart1Send((char *) HARD);
    Wait_ms(100);
#else
#error	"No Hardware defined in hard.h file"
#endif

#ifdef SOFT
    Usart1Send((char *) SOFT);
    Wait_ms(100);
#else
#error	"No Soft Version defined in hard.h file"
#endif

    //--- INICIO PROGRAMA DE PRODUCCION ---//
    //reset a la SM del display
    Display_ResetSM();
    //apago el display
    Display_ShowNumbers(DISPLAY_NONE);
    
    BuzzerCommands(BUZZER_LONG_CMD, 2);

    AudioInit ();

    // check memory jedec
    if (readJEDEC())
        Usart1Send("Memory JEDEC is good\r\n");
    else
        Usart1Send("Memory WRONG!\r\nContinue anyway\r\n");        

    ShowFileSystem();

    //reviso si esta grabada la memoria
    if (files.posi0 == 0xFFFFFFFF)
    {
        //memoria no grabada
        Display_ShowNumbers(DISPLAY_ERROR);
        BuzzerCommands(BUZZER_LONG_CMD, 10);
        while (1)
            UpdateBuzzer();
    }

    LoadConfiguration();
    //reviso configuracion de memoria
    if (param_struct.b1t == 0xFF)
    {
        //memoria no configurada
        Display_ShowNumbers(DISPLAY_ERROR2);
        BuzzerCommands(BUZZER_LONG_CMD, 10);
        while (1)
            UpdateBuzzer();
    }
    ShowConfiguration();

    //si esta todo bien configurado prendo el led
    LED_ON;

    char str [40];    
    main_state_e main_state = 0;
    unsigned char switches, switches_posi0, switches_posi1, switches_posi2;
    unsigned short position, mass_erase_position;
    unsigned char keypad_locked = 1;
    unsigned char digit_remote = 0;
    unsigned char remote_is_working = 0;
    unsigned char result;
    unsigned short seq_number = 0;
    unsigned short code_position;
    unsigned int code;    
    
    while (1)
    {
        switch (main_state)
        {
        case MAIN_INIT:

            switches_posi0 = 0;
            switches_posi1 = 0;
            switches_posi2 = 0;
            position = 0;
            //code = 0;

#ifdef CON_BLOQUEO_DE_KEYPAD
            if (!interdigit_timeout)
            {
                if (keypad_locked)
                    main_state = MAIN_MAIN;
                else    //cambio 21-09-18 a unica forma de hacer un lock es estar 60s sin tocar nada
                    main_state = MAIN_TO_UNLOCK;
            }
#else
            main_state = MAIN_UNLOCK;
#endif
            break;

        case MAIN_MAIN:
            if (CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position) == KNUMBER_FINISH)
            {
                if (position == 951)
                {
                    keypad_locked = 0;
                    Usart1Send("Master Unlock\r\n");
                    main_state = MAIN_TO_UNLOCK;
                }
                else
                {
                    unsigned short code_in_mem = SST_CheckIndexInMemory(1000);

                    if ((code_in_mem == 0xFFFF) && (position == 416))
                    {
                        Usart1Send("User default Unlock\r\n");
                        main_state = MAIN_TO_UNLOCK;
                        keypad_locked = 0;
                    }

                    if (position == code_in_mem)                        
                    {
                        Usart1Send("User Unlock\r\n");                        
                        main_state = MAIN_TO_UNLOCK;
                        keypad_locked = 0;
                    }
                }
                //position = (unsigned short) SST_CheckIndexInMemory(1000);
                //sprintf(str, "user code: %03d\r\n", position);
                //Usart1Send(str);
            }

            //ahora reviso si hay algun control enviando y si es el remote_keypad o un control de alarma
            switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
            if (switches == RK_MUST_BE_CONTROL)
            {
                //TODO: codigo original OK
                alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
                main_state = MAIN_IN_ALARM;
                Charger_Disconnect_For_Siren();
                //FIN: codigo original OK

                //TODO: codigo nuevo a probar
                // no funciona bien, ver mas abajo
                // unsigned short code_position = 0;
                // unsigned char button = 0;
                // code_position = CheckBaseCodeInMemory(code);
                // if ((code_position >= 0) && (code_position <= 1023))
                //     button = SST_CheckButtonInCode(code);

                // if ((button > 0) && (button <= 4))
                // {
                //     Func_Alarm_Reset_SM();
                //     new_global_position = code_position;
                //     new_global_button = button;
                //     main_state = MAIN_IN_ALARM;
                // }
                //FIN: codigo nuevo a probar
                
            }
            else if (switches == RK_NUMBER_FINISH)
            {
                if (position == 951)
                {
                    Usart1Send("Master Remote Unlock\r\n");
                    main_state = MAIN_TO_UNLOCK;
                    unlock_by_remote = 1;
                    keypad_locked = 0;
                }
                else
                {
                    unsigned short code_in_mem = SST_CheckIndexInMemory(1000);

                    if ((code_in_mem == 0xFFFF) && (position == 416))
                    {
                        Usart1Send("User default Remote Unlock\r\n");
                        main_state = MAIN_TO_UNLOCK;
                        unlock_by_remote = 1;
                        keypad_locked = 0;
                    }

                    if (position == code_in_mem)
                    {
                        Usart1Send("User Remote Unlock\r\n");
                        main_state = MAIN_TO_UNLOCK;
                        unlock_by_remote = 1;
                        keypad_locked = 0;
                    }
                }
            }

            // check at last for a SMS activation
            if (CheckSMS())
            {
                alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
                main_state = MAIN_IN_ALARM;
                Charger_Disconnect_For_Siren();
            }
            
            break;

        case MAIN_TO_UNLOCK:
            timer_keypad_enabled = 60000;

            //si se esta mostrando algo espero
            if (Display_IsFree())
            {
                Display_ShowNumbers(DISPLAY_REMOTE);
                digit_remote = 1;
            }
            else
                digit_remote = 0;

            main_state = MAIN_UNLOCK;
            break;

        case MAIN_UNLOCK:
            //este es la caso principal de selcciones desde aca segun lo elegido
            //voy pasando a otros casos

            switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
            if (switches == KCANCEL)
            {
                //se cancelo y no debo ir a ningun lado, me quedo aca
                digit_remote = 0;
            }

            if (switches == KNUMBER_FINISH)
            {
                if (position == 800)
                {
                    Display_ShowNumbers(DISPLAY_PROG);
                    main_state = MAIN_TO_CHANGE_USER_PASSWORD;
                    Usart1Send("Change User Password\r\n");
                    wait_for_code_timeout = param_struct.wait_for_code;
                }
                else
                {
                    Display_ShowNumbers(DISPLAY_LINE);
                    main_state = MAIN_TO_SAVE_AT_LAST;
                    wait_for_code_timeout = param_struct.wait_for_code;
                    sprintf(str, "Guardar en: %03d\r\n", position);
                    Usart1Send(str);
                }
            }

            if (!digit_remote)
            {
                if (Display_IsFree())
                {
                    Display_ShowNumbers(DISPLAY_REMOTE);
                    digit_remote = 1;
                }
            }


            //ahora reviso si hay algun control enviando y si es el remote_keypad o un control de alarma
            switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
            if (switches == RK_MUST_BE_CONTROL)
            {
                //TODO: codigo original OK
                alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
                main_state = MAIN_IN_ALARM;
                Charger_Disconnect_For_Siren();                
                //FIN: codigo original OK

                //TODO: codigo nuevo a probar
                // no funciona bien, ver mas abajo
                // unsigned short code_position = 0;
                // unsigned char button = 0;
                // code_position = CheckBaseCodeInMemory(code);
                // if ((code_position >= 0) && (code_position <= 1023))
                //     button = SST_CheckButtonInCode(code);

                // if ((button > 0) && (button <= 4))
                // {
                //     Func_Alarm_Reset_SM();
                //     new_global_position = code_position;
                //     new_global_button = button;
                //     main_state = MAIN_IN_ALARM;
                // }
                //FIN: codigo nuevo a probar
                
            }
            else if (switches == RK_NUMBER_FINISH)
            {
                remote_is_working = 1;
                wait_for_code_timeout = param_struct.wait_for_code;
                sprintf(str, "Keypad Remoto guardar en: %03d\r\n", position);
                Usart1Send(str);
                main_state = MAIN_TO_SAVE_AT_LAST;                
            }
            else if (switches == RK_CANCEL)
                digit_remote = 0;
            

#ifdef CON_BLOQUEO_DE_KEYPAD
            if (!timer_keypad_enabled)
            {
                keypad_locked = 1;
                Usart1Send("Keypad Locked\r\n");
                main_state = MAIN_MAIN;
                unlock_by_remote = 0;
                remote_is_working = 0;
                Display_ShowNumbers(DISPLAY_NONE);
            }
#endif
            // check at last for a SMS activation
            if (CheckSMS())
            {
                alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
                main_state = MAIN_IN_ALARM;
                Charger_Disconnect_For_Siren();
            }
            
            break;

        case MAIN_TO_CHANGE_USER_PASSWORD:
            switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
            if (switches == KNUMBER_FINISH)
            {
                SST_WriteCodeToMemory(1000, position);
                Usart1Send("User Password changed\r\n");
                BuzzerCommands(BUZZER_SHORT_CMD, 7);
                main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            }

            if (switches == KCANCEL)
            {
                main_state = MAIN_TO_MAIN_CANCEL;
            }

            if (!wait_for_code_timeout)
                main_state = MAIN_TO_MAIN_TIMEOUT;

            break;

        case MAIN_IN_ALARM:
#ifndef PRODUCCION_CHICKEN_BUCLE            
            //TODO: version anterior que funciona
            // check if we get here from sms or control
            if (CheckSMS())
            {
                result = FuncAlarm(1);    //sms activation only needs one kick
                ResetSMS();
            }
            else
                result = FuncAlarm(0);
            
            if (result == END_OK)
            {
                main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            }
            //FIN: version anterior que funciona
#endif
            
#ifdef PRODUCCION_CHICKEN_BUCLE
            result = FuncAlarmChickenBucle(0);
            
            if (result == END_OK)
            {
                main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            }            
#endif

            //TODO: nueva version FuncAlarm
            // no funciona activacion con botones B2 al B4
            // activa con B1 pero nunca sale de sirena, no pasa audios ni termina
            // falta timer???
            // if (CheckSMS())
            // {
            //     //llegue a la alarma por SMS, doy el primer kick
            //     result = Func_Alarm_SM(SMS_ALARM, 0, 0);
            //     ResetSMS();
            // }
            // else
            // {
            //     //Reviso si hay nuevo control, si no estoy pasando audio
            //     unsigned char button = 0;
            //     unsigned int code = 0;
            //     unsigned short code_position = 0;
                
            //     if (audio_state == AUDIO_INIT)
            //         button = CheckForButtons(&code_position, &code);
                
            //     if (button)
            //     {
            //         new_global_position = code_position;
            //         new_global_button = button;
            //         result = Func_Alarm_SM(NEWCODE_ALARM, new_global_position, new_global_button);
            //     }
            //     else
            //     {
            //         result = Func_Alarm_SM(CONTROL_ALARM, new_global_position, new_global_button);
            //     }
            // }

            // if (result == resp_ok)
            // {
            //     main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            // }            
            //FIN: nueva version FuncAlarm
            break;

        case MAIN_TO_SAVE_AT_LAST:

            //me quedo esperando un código de control valido para guardarlo en posi
            mass_erase_position = position;
            switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);

            //reviso primero todas las posibilidades del teclado remoto
            if (remote_is_working)
            {
                if (switches == RK_CANCEL)
                    main_state = MAIN_TO_MAIN_CANCEL;

                //se eligio borrar una posicion de control desde el teclado remoto
                //xxx# y 0#
                else if ((switches == RK_NUMBER_FINISH) && (position == 0))	//termino el numero y era 0
                {
                    //se va a borrar la posicion con teclado remoto
                    position = mass_erase_position;	//update de posicion del control
                    main_state = MAIN_TO_DEL_CODE;
                }

                //se eligio entrar en grabado de controles con secuencia desde el teclado remoto
                //dos veces el mismo codigo de grabado
                else if ((switches == RK_NUMBER_FINISH) && (position == mass_erase_position))
                {
                    sprintf(str, "Grabar en secuencia remota desde: %d\r\n", position);
                    Usart1Send(str);
                    Display_ShowNumbers(DISPLAY_S);
                    SirenCommands(SIREN_CONFIRM_OK_CMD);
                    seq_number = 0;                
                    main_state = MAIN_TO_SAVE_IN_SEQUENCE;                
                }
            }

            //ahora podria ser un control
            if (switches == RK_MUST_BE_CONTROL)
            {
                code = code0;
                code <<= 16;
                code |= code1;
                code_position = CheckCodeInMemory(code);
                if (code_position == 0xFFFF)
                {
                    //el codigo no se habia utilizado
                    if (Write_Code_To_Memory(position, code) != 0)
                        Usart1Send((char *) "Codigo Guardado OK\r\n");
                    else
                        Usart1Send((char *) "Error al guardar\r\n");

                    Display_ConvertPosition(position);
                    BuzzerCommands(BUZZER_SHORT_CMD, 7);
                    if (remote_is_working)
                        SirenCommands(SIREN_CONFIRM_OK_CMD);                    

                }
                else
                {
                    //se habia utilizado en otra posicion
                    sprintf(str, "Error codigo ya esta en: %03d\r\n", code_position);
                    Usart1Send(str);

                    Display_ConvertPosition(code_position);
                    BuzzerCommands(BUZZER_HALF_CMD, 2);
                    if (remote_is_working)
                        SirenCommands(SIREN_HALF_CMD);
                    
                }
                main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            }

            if (!remote_is_working)
            {
                //Backup de la posicion del control que me trajo hasta aca
                mass_erase_position = position;
                switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
                if (switches == KCANCEL)
                    main_state = MAIN_TO_MAIN_CANCEL;

                //se eligio borrar una posicion de control
                //xxx# y 0#
                if ((switches == KNUMBER_FINISH) && (position == 0))
                {
                    position = mass_erase_position;	//update de posicion del control
                    main_state = MAIN_TO_DEL_CODE;
                    switches = KNONE;
                }

                //se eligio borrar todos los codigos de memoria (BLANQUEO COMPLETO)
                //000# y luego 999#
                if ((switches == KNUMBER_FINISH) && (position == 999) && (mass_erase_position == 0))
                {
                    Usart1Send((char *) "\r\n- CUIDADO entrando en Blanqueo Completo -\r\n");
                    main_state = MAIN_TO_MASS_ERASE_AT_LAST;
                    switches = KNONE;
                }

                //se eligio entrar en grabado de controles con secuencia
                //dos veces el mismo codigo de grabado
                if ((switches == KNUMBER_FINISH) && (position == mass_erase_position))
                {
                    sprintf(str, "Grabar en secuencia desde: %d\r\n", position);
                    Usart1Send(str);
                    Display_ShowNumbers(DISPLAY_S);
                    seq_number = 0;                
                    main_state = MAIN_TO_SAVE_IN_SEQUENCE;
                    switches = KNONE;
                }
            }

            if (!wait_for_code_timeout)
                main_state = MAIN_TO_MAIN_TIMEOUT;

            break;

        case MAIN_TO_DEL_CODE:
            if (Write_Code_To_Memory(position, 0xFFFFFFFF) != 0)
            {
                Usart1Send((char *) "Codigo Borrado OK\r\n");
                Display_VectorToStr("0.");
                BuzzerCommands(BUZZER_SHORT_CMD, 7);
                if (remote_is_working)
                    SirenCommands(SIREN_CONFIRM_OK_CMD);                    
            }
            else
            {
                Usart1Send((char *) "Error al borrar\r\n");
                Display_ShowNumbers(DISPLAY_NONE);
                BuzzerCommands(BUZZER_HALF_CMD, 1);
                if (remote_is_working)
                    SirenCommands(SIREN_HALF_CMD);                    
            }

            main_state = MAIN_TO_MAIN_OK;
            break;

        case MAIN_TO_MASS_ERASE_AT_LAST:
            //se va a borrar la memoria
            if (EraseAllMemory() != 0)
            {
                Usart1Send((char *) "Memoria Completa Borrada OK\r\n");
                Display_VectorToStr("0.");
                BuzzerCommands(BUZZER_SHORT_CMD, 7);
                main_state = MAIN_TO_MAIN_WAIT_5SEGS;
            }
            else
            {
                Usart1Send((char *) "Error al borrar memoria\r\n");
                Display_ShowNumbers(DISPLAY_NONE);
                BuzzerCommands(BUZZER_HALF_CMD, 1);
                main_state = MAIN_TO_MAIN_CANCEL;
            }
            break;

        case MAIN_TO_SAVE_IN_SEQUENCE:
            //me quedo esperando un código de control valido para guardarlo en posi
            switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
            if (switches == RK_MUST_BE_CONTROL)
            {
                code = code0;
                code <<= 16;
                code |= code1;
                code_position = CheckCodeInMemory(code);
                if (code_position == 0xFFFF)
                {
                    //el codigo no se habia utilizado
                    if (Write_Code_To_Memory((position + seq_number), code) != 0)
                    {
                        sprintf(str, "Codigo Guardado OK en: %d\r\n", (position + seq_number));
                        Usart1Send(str);
                        Display_ConvertPosition((position + seq_number));
                        BuzzerCommands(BUZZER_SHORT_CMD, 7);                        
                        seq_number++;
                        main_state = MAIN_TO_SAVE_IN_SEQUENCE_WAITING;
                        if (remote_is_working)
                            SirenCommands(SIREN_CONFIRM_OK_CMD);                    
                        
                    }
                    else
                    {
                        Usart1Send((char *) "Error al guardar, problemas de memoria??\r\n");
                        //salgo por error
                        Display_VectorToStr("e");
                        main_state = MAIN_TO_MAIN_WAIT_5SEGS;                        
                    }
                }
                else
                {
                    //se habia utilizado en otra posicion
                    sprintf(str, "Error codigo ya esta en: %03d\r\n", code_position);
                    Usart1Send(str);
                    Display_ConvertPosition(code_position);
                    BuzzerCommands(BUZZER_HALF_CMD, 2);
                    //salgo por error
                    main_state = MAIN_TO_MAIN_WAIT_5SEGS;
                    if (remote_is_working)
                        SirenCommands(SIREN_HALF_CMD);
                    
                }
            }

            if (!remote_is_working)
            {
                switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
                if (switches == KCANCEL)
                {
                    Usart1Send((char *) "Termino de grabar en secuencia\r\n");
                    main_state = MAIN_TO_MAIN_CANCEL;
                }
            }
            else if (switches == RK_CANCEL)
            {
                SirenCommands(SIREN_CONFIRM_OK_CMD);
                Usart1Send((char *) "Termino de grabar remotos en secuencia\r\n");
                main_state = MAIN_TO_MAIN_CANCEL;                
            }

            //si no estoy en enviado la secuencia de numeros pongo la S
            if (Display_IsFree())
                Display_ShowNumbers(DISPLAY_S);
            
            break;
            
        case MAIN_TO_SAVE_IN_SEQUENCE_WAITING:
            if (Display_IsFree())
                main_state = MAIN_TO_SAVE_IN_SEQUENCE;
            
            break;
            
        case MAIN_TO_MAIN_CANCEL:
            Usart1Send((char *) "Opereta cancelada\r\n");
            main_state = MAIN_INIT;
            interdigit_timeout = 300;	//espero que se limpien las teclas
            remote_is_working = 0;
            break;

        case MAIN_TO_MAIN_TIMEOUT:
            Usart1Send((char *) "Timeout\r\n");
            main_state = MAIN_INIT;
            interdigit_timeout = 300;	//espero que se limpien las teclas
            remote_is_working = 0;            
            break;

        case MAIN_TO_MAIN_WAIT_5SEGS:
            interdigit_timeout = ACT_DESACT_IN_MSECS;	//espero 5 segundos luego del codigo grabado OK
            main_state = MAIN_TO_MAIN_WAIT_5SEGSA;
            remote_is_working = 0;

            Charger_Connect ();
            break;

        case MAIN_TO_MAIN_WAIT_5SEGSA:
            if (!interdigit_timeout)
                main_state = MAIN_TO_MAIN_OK;
            break;

        case MAIN_TO_MAIN_OK:
            remote_is_working = 0;
            main_state = MAIN_INIT;
            break;

        // case MAIN_MEMORY_DUMP:
        //     //cargo 256 numeros al vector de 1024 posiciones
        //     SST_MemoryDump (OFFSET_CODES_256);
        //     main_state = MAIN_INIT;
        //     break;

        // case MAIN_MEMORY_DUMP2:
        //     //cargo 256 numeros al vector de 1024 posiciones
        //     SST_MemoryDump (OFFSET_CODES_512);
        //     main_state = MAIN_INIT;
        //     break;

        default:
            main_state = MAIN_INIT;
            break;

        }

        UpdateBuzzer();

        Display_UpdateSM();

        UpdateSiren();

        UpdateAudio();

        Battery_Update ();

        CommsUpdate ();

        // check if monitoring is needed, leave only on board reset
        if (Production_Check_Monitoring ())
        {
            Usart1Send("going to 115200\r\n");
            
            Wait_ms(100);
            
            Usart1ChangeBaud(USART_115200);    
            
            FuncGestion();
        }
    }
}


//funcion de alarmas, revisa codigo en memoria y actua en consecuencia
unsigned char FuncAlarm (unsigned char sms_alarm)
{
    unsigned char button;
    unsigned int code;
    unsigned short code_position;

    char str[50];

    switch (alarm_state)
    {
    case ALARM_START:
        alarm_state = ALARM_NO_CODE;
        code = code0;
        code <<= 16;
        code |= code1;

        if (sms_alarm)
        {
            Usart1Send("SMS Activo: 911 B1\r\n");
            last_one_or_three = 911;
            alarm_state = ALARM_BUTTON1;
            repetition_counter = param_struct.b1r;
            button_timer_secs = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15
#ifdef USE_F12_PLUS_WITH_SM
            //modificacion 24-01-2019 F12PLUS espera 10 segundos y se activa 5 segundos
            F12_State_Machine_Start();
#endif
#ifdef USE_BUTTON4_ONLY_5MINS
	    button4_only5mins_timeout = BUTTON4_5MINS_TT;
#endif                        
        }
        else
        {
            //code_position = CheckCodeInMemory(code);
            code_position = CheckBaseCodeInMemory(code);

            if ((code_position >= 0) && (code_position <= 1023))
            {
                sprintf(str, (char *) "Activo: %03d ", code_position);
                //el codigo existe en memoria
                //reviso el boton
                button = SST_CheckButtonInCode(code);
                if (button == 1)
                {
                    last_one_or_three = code_position;
                    alarm_state = ALARM_BUTTON1;
                    strcat(str, (char *) "B1\r\n");
                    repetition_counter = param_struct.b1r;
#ifdef USE_F12_PLUS_WITH_SM
                    //modificacion 24-01-2019 F12PLUS espera 10 segundos y se activa 5 segundos
                    F12_State_Machine_Start();
#endif
#ifdef USE_BUTTON4_ONLY_5MINS
		    button4_only5mins_timeout = BUTTON4_5MINS_TT;
#endif
                }
                else if (button == 2)
                {
                    //original boton 2
                    last_two = code_position;
                    alarm_state = ALARM_BUTTON2;
                    strcat(str, (char *) "B2\r\n");
                }
                else if (button == 3)
                {
                    last_one_or_three = code_position;
                    alarm_state = ALARM_BUTTON3;
                    strcat(str, (char *) "B3\r\n");
                    repetition_counter = param_struct.b3r;
#ifdef USE_BUTTON4_ONLY_5MINS
		    button4_only5mins_timeout = BUTTON4_5MINS_TT;
#endif
                }
                else if (button == 4)
                {
                    alarm_state = ALARM_BUTTON4;
                    strcat(str, (char *) "B4\r\n");
                    repetition_counter = param_struct.b4r;
                }

                //button_timer_secs = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
                button_timer_secs = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15

                Usart1Send(str);
            }
        }
        break;

    case ALARM_BUTTON1:
        FPLUS_ON;
#ifdef USE_F12_PLUS_ON_BUTTON1
        F12PLUS_ON;
#endif

        SirenCommands(SIREN_MULTIPLE_UP_CMD);
        alarm_state++;
        break;

    case ALARM_BUTTON1_A:
        if (!button_timer_secs)
        {
            button_timer_secs = param_struct.b1t;
            alarm_state++;
        }
        break;

    case ALARM_BUTTON1_B:
        //me quedo esperando que apaguen o timer
#ifdef PRODUCCION_NORMAL
        if (CheckForButtons(&code_position, &code) == 1)	//reviso solo boton 1
        {
            alarm_state = ALARM_BUTTON1_FINISH;
        }
#endif

#ifdef PRODUCCION_BUCLE
        if (CheckForButtons(&code_position, &code) == 4)	//desactivo solo con 4
        {
            alarm_state = ALARM_BUTTON1_FINISH;
        }
#endif
        
        if (!button_timer_secs)
        {
            //tengo timeout, avanzo al audio
            alarm_state++;
        }
        break;

    case ALARM_BUTTON1_C:
        //paso el audio y descuento un ciclo
        if (repetition_counter > 1)
        {
            repetition_counter--;
            SirenCommands(SIREN_STOP_CMD);

            if (param_struct.audio_buttons & B1_AUDIO_MASK)
                VectorToSpeak('a');
        
            PositionToSpeak(last_one_or_three);
            alarm_state++;
        }
        else
        {
            Usart1Send((char*) "Timeout B1 ");
            alarm_state = ALARM_BUTTON1_FINISH;
        }
        break;

    case ALARM_BUTTON1_D:
        if (Get_Audio_Init())
        {
            //termino de enviar audio
            alarm_state = ALARM_BUTTON1;
        }
        break;

    case ALARM_BUTTON1_FINISH:
        sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
        Usart1Send(str);

        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
#ifdef USE_F12_PLUS_WITH_SM
        F12_State_Machine_Reset();
#else
        F12PLUS_OFF;
#endif

        PositionToSpeak(last_one_or_three);
        alarm_state++;
        break;

    case ALARM_BUTTON1_FINISH_B:
        if (Get_Audio_Init())        
        {
            //termino de enviar audio
            return END_OK;
        }
        break;

    case ALARM_BUTTON2:		//solo enciendo reflectores
        FPLUS_ON;

        alarm_state++;
#ifdef HT6P20B2_FOURTH_BUTTON
        button_timer_secs = 4;	//4 segundos overdrive
#endif
        break;

    case ALARM_BUTTON2_A:		//espero los primeros 2 segundos
#ifdef HT6P20B2_FOURTH_BUTTON
        if (!button_timer_secs)
        {
            button_timer_secs = 2;
            alarm_state++;
        }
#else
        alarm_state++;
#endif
        break;

    case ALARM_BUTTON2_B:
#ifdef HT6P20B2_FOURTH_BUTTON
        if (CheckForButtons(&code_position, &code) == 2)	//reviso solo boton 2 para simular el 4
        {
            if (code_position == last_two)
                alarm_state = ALARM_BUTTON2_D;
        }
#endif
        if (!button_timer_secs)
        {
            button_timer_secs = param_struct.b2t;
            alarm_state++;
        }
        break;

    case ALARM_BUTTON2_C:
        //me quedo esperando que apaguen o timer

        button = CheckForButtons(&code_position, &code);
#ifdef PRODUCCION_NORMAL
        if (button == 2)	//reviso solo boton 2
        {
            sprintf(str, "Desactivo: %03d\r\n", code_position);
            Usart1Send(str);
            alarm_state = ALARM_BUTTON2_FINISH;
        }

        //reviso el boton1 o sms
        if ((button == 1) || (sms_alarm))
        {
            if (sms_alarm)
            {
                strcpy(str, "SMS Activo: 911 B1\r\n");
                last_one_or_three = 911;
            }
            else
            {
                sprintf(str, "Activo: %03d B1\r\n", code_position);
                last_one_or_three = code_position;
            }
            
            Usart1Send(str);
            repetition_counter = param_struct.b1r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON1;
        }

        if (button == 3)		//reviso el boton
        {
            sprintf(str, "Activo: %03d B3\r\n", code_position);
            Usart1Send(str);

            last_one_or_three = code_position;
            repetition_counter = param_struct.b3r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON3;
        }
#endif
#ifdef PRODUCCION_BUCLE
        if (button == 1)		//reviso el boton
        {
            sprintf(str, "Activo: %03d B1\r\n", code_position);
            Usart1Send(str);

            last_one_or_three = code_position;
            repetition_counter = param_struct.b1r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON1;
        }

        if (button == 3)		//reviso el boton
        {
            sprintf(str, "Activo: %03d B3\r\n", code_position);
            Usart1Send(str);

            last_one_or_three = code_position;
            repetition_counter = param_struct.b3r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON3;
        }

        if (button == 4)	//reviso boton 4 para desactivar
        {
            sprintf(str, "Desactivo: %03d\r\n", code_position);
            Usart1Send(str);
            alarm_state = ALARM_BUTTON2_FINISH;
        }
#endif

        if (!button_timer_secs)
        {
            //tengo timeout, apago reflectores
            Usart1Send((char*) "Timeout B2\r\n");
            alarm_state = ALARM_BUTTON2_FINISH;
        }
        break;

    case ALARM_BUTTON2_D:
        SirenCommands(SIREN_STOP_CMD);
        PositionToSpeak(last_one_or_three);
        alarm_state++;
        break;

    case ALARM_BUTTON2_E:
        if (Get_Audio_Init())
        {
            //termino de enviar audio
            alarm_state++;
        }
        break;

    case ALARM_BUTTON2_FINISH:
        FPLUS_OFF;
        return END_OK;
        break;

    case ALARM_BUTTON3:
        FPLUS_ON;
        //SirenCommands(SIREN_SINGLE_CMD);
        SirenCommands(SIREN_SINGLE_CHOPP_CMD);
        //button_timer_secs = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
        alarm_state++;
        break;

    case ALARM_BUTTON3_A:
        if (!button_timer_secs)
        {
            button_timer_secs = param_struct.b3t;
            alarm_state++;
        }
        break;

    case ALARM_BUTTON3_B:
        //me quedo esperando que apaguen o timer

        button = CheckForButtons(&code_position, &code);
#ifdef PRODUCCION_NORMAL
        if (button == 3)	//reviso solo boton 3
        {
            sprintf(str, "Desactivo: %03d\r\n", code_position);
            Usart1Send(str);
            alarm_state = ALARM_BUTTON3_FINISH;
        }

        //reviso el boton1 o sms
        if ((button == 1) || (sms_alarm))
        {
            if (sms_alarm)
            {
                strcpy(str, "SMS Activo: 911 B1\r\n");
                last_one_or_three = 911;
            }
            else
            {
                sprintf(str, "Activo: %03d B1\r\n", code_position);
                last_one_or_three = code_position;
            }
        
            Usart1Send(str);
            repetition_counter = param_struct.b1r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON1;
        }
#endif

#ifdef PRODUCCION_BUCLE
        if (button == 1)		//reviso el boton
        {
            sprintf(str, "Activo: %03d B1\r\n", code_position);
            Usart1Send(str);

            last_one_or_three = code_position;
            repetition_counter = param_struct.b1r;
            button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_state = ALARM_BUTTON1;
        }

        if (button == 4)	//reviso solo boton 4
        {
            sprintf(str, "Desactivo: %03d\r\n", code_position);
            Usart1Send(str);
            alarm_state = ALARM_BUTTON3_FINISH;
        }
#endif

        if (!button_timer_secs)
        {
            //tengo timeout, avanzo al audio

            alarm_state++;
        }
        break;

    case ALARM_BUTTON3_C:
        // descuento un ciclo y paso el audio
        if (repetition_counter > 1)
        {
            repetition_counter--;
            SirenCommands(SIREN_STOP_CMD);

            if (param_struct.audio_buttons & B3_AUDIO_MASK)
                VectorToSpeak('b');
        
            PositionToSpeak(last_one_or_three);
            alarm_state++;
        }
        else
        {
            Usart1Send((char*) "Timeout B3 ");
            alarm_state = ALARM_BUTTON3_FINISH;
        }
        break;

    case ALARM_BUTTON3_D:
        if (Get_Audio_Init())        
        {
            //termino de enviar audio
            alarm_state = ALARM_BUTTON3;
        }
        break;

    case ALARM_BUTTON3_FINISH:
        sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
        Usart1Send(str);

        SirenCommands(SIREN_STOP_CMD);
//			F12PLUS_OFF;
        FPLUS_OFF;

        PositionToSpeak(last_one_or_three);
        alarm_state++;
        break;

    case ALARM_BUTTON3_FINISH_B:
        if (Get_Audio_Init())
        {
            //termino de enviar audio
            return END_OK;
        }
        break;

    case ALARM_BUTTON4:
#ifdef BUTTON4_5MINS_TT
	SirenCommands(SIREN_STOP_CMD);

	if (button4_only5mins_timeout)
	    PositionToSpeak(last_one_or_three);
	
	alarm_state++;
#else
        SirenCommands(SIREN_STOP_CMD);
	PositionToSpeak(last_one_or_three);
	alarm_state++;
#endif
        break;

    case ALARM_BUTTON4_A:
        if (Get_Audio_Init())        
        {
            //termino de enviar audio
            alarm_state++;
        }
        break;

    case ALARM_BUTTON4_FINISH:
        return END_OK;
        break;

    case ALARM_NO_CODE:
        return END_OK;
        break;
    default:
        alarm_state = 0;
        break;
    }

#ifdef USE_F12_PLUS_WITH_SM
    F12_State_Machine();
#endif

    return WORKING;
}


#ifdef PRODUCCION_CHICKEN_BUCLE
//funcion de alarmas, revisa codigo en memoria y actua en consecuencia
unsigned char FuncAlarmChickenBucle (unsigned char sms_alarm)
{
    unsigned char button;
    unsigned int code;
    unsigned short code_position;

    char str[50];

    switch (alarm_state)
    {
    case ALARM_START:
        alarm_state = ALARM_NO_CODE;
        code = code0;
        code <<= 16;
        code |= code1;

        if (sms_alarm)
        {
            Usart1Send("SMS Not supported\r\n");
        }
        else
        {
            //code_position = CheckCodeInMemory(code);
            code_position = CheckBaseCodeInMemory(code);

            if ((code_position >= 0) && (code_position <= 1023))
            {
                sprintf(str, (char *) "Activo: %03d ", code_position);
                //el codigo existe en memoria
                //reviso el boton
                button = SST_CheckButtonInCode(code);
                if (button == 1)
                {
                    last_one_or_three = code_position;
                    alarm_state = ALARM_BUTTON1;
                    strcat(str, (char *) "B1\r\n");
                    repetition_counter = param_struct.b1r;
#ifdef USE_F12_PLUS_WITH_SM
                    //modificacion 24-01-2019 F12PLUS espera 10 segundos y se activa 5 segundos
                    F12_State_Machine_Start();
#endif

                }
                else if (button == 2)
                {
                    //original boton 2
                    last_two = code_position;
                    alarm_state = ALARM_BUTTON2;
                    strcat(str, (char *) "B2\r\n");
                    repetition_counter = param_struct.b2r;                    
                }
                else if (button == 3)
                {
                    last_one_or_three = code_position;
                    alarm_state = ALARM_BUTTON3;
                    strcat(str, (char *) "B3\r\n");
                    repetition_counter = param_struct.b3r;
                }
                else if (button == 4)
                {
                    alarm_state = ALARM_BUTTON4;
                    strcat(str, (char *) "B4\r\n");
                    repetition_counter = param_struct.b4r;
                }

                //button_timer_secs = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
                button_timer_secs = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15

                Usart1Send(str);
            }
        }
        break;

    case ALARM_BUTTON1:
        FPLUS_ON;
#ifdef USE_F12_PLUS_ON_BUTTON1
        F12PLUS_ON;
#endif

        SirenCommands(SIREN_MULTIPLE_UP_CMD);
        last_two = 0;
        alarm_state++;
        break;

    case ALARM_BUTTON1_A:
        if (!button_timer_secs)
        {
            button_timer_secs = 90;
            alarm_state++;
        }
        break;

    case ALARM_BUTTON1_B:
        //me quedo esperando que apaguen o timer
        if (CheckForButtons(&code_position, &code) == 4)	//desactivo solo con 4
        {
            alarm_state = ALARM_BUTTON1_D;
        }
        
        if (!button_timer_secs)
        {
            //tengo timeout, corto
            alarm_state++;
        }
        break;

    case ALARM_BUTTON1_C:
        Usart1Send((char*) "Timeout B1 ");

        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
#ifdef USE_F12_PLUS_WITH_SM
        F12_State_Machine_Reset();
#else
        F12PLUS_OFF;
#endif

        PositionToSpeak(last_one_or_three);
        alarm_state = ALARM_BUTTON1_FINISH;
        break;

    case ALARM_BUTTON1_D:
        sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
        Usart1Send(str);

        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
#ifdef USE_F12_PLUS_WITH_SM
        F12_State_Machine_Reset();
#else
        F12PLUS_OFF;
#endif

        PositionToSpeak(last_one_or_three);
        alarm_state++;
        break;

    case ALARM_BUTTON1_FINISH:
        if (Get_Audio_Init())
        {
            //termino de enviar audio
            return END_OK;
        }        
        break;

    case ALARM_BUTTON1_FINISH_B:
        break;

    case ALARM_BUTTON2:		//solo enciendo reflectores y sirena
        FPLUS_ON;
        // SirenCommands(SIREN_MULTIPLE_DOWN_CMD);
        SirenCommands(SIREN_SINGLE_CHOPP_CMD);        
        last_one_or_three = 0;
        alarm_state++;
        break;

    case ALARM_BUTTON2_A:		//espero los primeros 2 segundos
        if (!button_timer_secs)
        {
            // button_timer_secs = param_struct.b2t;
            button_timer_secs = 90;    //1.5 min         
            alarm_state++;
        }
        break;

    case ALARM_BUTTON2_B:
        //me quedo esperando que apaguen o timer
        if (CheckForButtons(&code_position, &code) == 4)	//desactivo solo con 4
        {
            alarm_state = ALARM_BUTTON2_D;
        }
        
        if (!button_timer_secs)
        {
            //tengo timeout, corto
            alarm_state++;
        }
        break;

    case ALARM_BUTTON2_C:
        Usart1Send((char*) "Timeout B2 ");
        
        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
        PositionToSpeak(last_two);
        alarm_state = ALARM_BUTTON2_FINISH;
        break;

    case ALARM_BUTTON2_D:
        sprintf(str, "Desactivo: %03d\r\n", last_two);
        Usart1Send(str);

        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
        PositionToSpeak(last_two);
        alarm_state++;
        break;

    case ALARM_BUTTON2_E:
        alarm_state++;
        break;

    case ALARM_BUTTON2_FINISH:
        if (Get_Audio_Init())        
        {
            //termino de enviar audio
            return END_OK;
        }
        break;

    case ALARM_BUTTON3:
        FPLUS_ON;
        SirenCommands(SIREN_SINGLE_CHOPP_SMALL_CMD);
        last_two = 0;        
        alarm_state++;
        break;

    case ALARM_BUTTON3_A:
        if (!button_timer_secs)
        {
            button_timer_secs = 90;
            alarm_state++;
        }
        break;

    case ALARM_BUTTON3_B:
        //me quedo esperando que apaguen o timer
        if (CheckForButtons(&code_position, &code) == 4)	//desactivo solo con 4
        {
            alarm_state = ALARM_BUTTON3_D;
        }
        
        if (!button_timer_secs)
        {
            //tengo timeout, corto
            alarm_state++;
        }
        break;

    case ALARM_BUTTON3_C:
        Usart1Send((char*) "Timeout B3 ");
        
            // alarm_state = ALARM_BUTTON3_FINISH;
        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
        PositionToSpeak(last_one_or_three);
        alarm_state = ALARM_BUTTON3_FINISH;
        break;

    case ALARM_BUTTON3_D:
        sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
        Usart1Send(str);

        SirenCommands(SIREN_STOP_CMD);
        FPLUS_OFF;
        PositionToSpeak(last_one_or_three);
        alarm_state++;
        break;

    case ALARM_BUTTON3_FINISH:
        if (Get_Audio_Init())
        {
            //termino de enviar audio
            return END_OK;
        }
        break;

    case ALARM_BUTTON3_FINISH_B:
        break;

    case ALARM_BUTTON4:
        SirenCommands(SIREN_STOP_CMD);
        if (last_one_or_three)
            PositionToSpeak(last_one_or_three);
        else if (last_two)
            PositionToSpeak(last_two);
        
        alarm_state++;
        break;

    case ALARM_BUTTON4_A:
        if (Get_Audio_Init())        
        {
            //termino de enviar audio
            alarm_state++;
        }
        break;

    case ALARM_BUTTON4_FINISH:
        return END_OK;
        break;

    case ALARM_NO_CODE:
        return END_OK;
        break;
    default:
        alarm_state = 0;
        break;
    }

#ifdef USE_F12_PLUS_WITH_SM
    F12_State_Machine();
#endif

    return WORKING;
}
#endif    //PRODUCCION_CHICKEN_BUCLE


unsigned char on_monitoring = 0;
unsigned char Production_Check_Monitoring (void)
{
    return on_monitoring;
}


void Production_Set_Monitoring (void)
{
    on_monitoring = 1;
}


void Production_Timeouts (void)
{
    if (pilot_code)
        pilot_code--;

    if (wait_for_code_timeout)
        wait_for_code_timeout--;

    if (timer_keypad_enabled)
        timer_keypad_enabled--;

    Siren_Timeouts ();

    Battery_Timeouts ();
    
#ifdef BUTTON4_5MINS_TT
    if (button4_only5mins_timeout)
	button4_only5mins_timeout--;
#endif

    //cuenta 1 segundo
    if (button_timer_internal)
        button_timer_internal--;
    else
    {
        if (button_timer_secs)
        {
            button_timer_secs--;
            button_timer_internal = 1000;
        }
    }
    
}
//--- end of file ---//
