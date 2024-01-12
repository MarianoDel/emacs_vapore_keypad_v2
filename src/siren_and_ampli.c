//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SIREN_AND_AMPLI.C ##################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "siren_and_ampli.h"
#include "hard.h"
#include "tim.h"
#include "usart.h"

#include "memory_conf.h"
#include "memory_utils.h"

#include <string.h>

// #include "stm32f0xx.h"
// #include "spi.h"
// #include "display_7seg.h"
// #include "parameters.h"

#define AC_PIN    1

#define LAST_NUMBER_SPEAK    5

//TIEMPOS DE LA SIRENA POLICIAL
//#define SIREN_FIRST_FREQ	800		//este es el tiempo en us (1/Hz) donde arranca la sirena
#define SIREN_FIRST_FREQ	1400		//este es el tiempo en us (1/Hz) donde arranca la sirena
#define SIREN_STEP_TIMEOUT  15		//tiempo en el que se mantiene una frecuencia antes de avanzar
#define SIREN_STEP_RELOAD	40
#define SIREN_FREQ_STEP		10		//incremento en us entre frecuencias (OJO usar siempre par, el pwm es /2 )
									//SIREN_FIRST_FREQ + SIREN_STEP_RELOAD * SIREN_FREQ_STEP = ultima frecuencia
									//freq original = 800 -> 1250Hz
									//800 + 50 * 20 = 1800 -> 555Hz

									//el tiempo en que tarda en llegar de una a la otra es:
									//SIREN_STEP_RELOAD * SIREN_STEP_TIMEOUT
									//50 * 80ms = 4s

#define SIREN_SHORT_TIMEOUT	200
#define SIREN_HALF_TIMEOUT	400
#define SIREN_LONG_TIMEOUT	1000


//ESTADOS DEL AUDIO
#define AUDIO_INIT					0
#define AUDIO_UPDATE				1
#define AUDIO_SENDING				2
#define AUDIO_CHECK_NEXT			3
#define AUDIO_FINISHING				4


// Externals -------------------------------------------------------------------
extern filesystem_typedef files;


// Global ----------------------------------------------------------------------
unsigned char siren_state = 0;
unsigned short freq_us = 0;
unsigned char siren_steps = 0;

volatile unsigned short siren_timeout;

//--- Audio Global Variables
#define FILE_OFFSET 44
unsigned char audio_state = 0;
unsigned char numbers_speak[LAST_NUMBER_SPEAK];
unsigned char * p_numbers_speak;
unsigned int * p_files_addr;
unsigned int * p_files_length;
unsigned int current_size = 0;

volatile short v_samples1[16];
volatile short v_samples2[16];
volatile unsigned char update_samples = 0;
volatile unsigned char buff_in_use = 1;


// Private Module Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void SirenCommands(unsigned char command)
{
    siren_state = command;
}


void UpdateSiren (void)
{
    switch (siren_state)
    {
    case SIREN_INIT:
        break;

    case SIREN_SINGLE:
        //cargo frecuencia y ciclo de trabajo
        //ChangeAmpli(FREQ_1000HZ, DUTY_50_1000);
        ChangeAmpli(FREQ_800HZ, DUTY_50_800);
        Power_Ampli_Ena();
        siren_state = SIREN_SINGLE_RINGING;
        break;

    case SIREN_SINGLE_RINGING:
        break;

    case SIREN_SINGLE_CHOPP_ON:
        if (!siren_timeout)
        {
            //cargo frecuencia y ciclo de trabajo
            //ChangeAmpli(FREQ_1000HZ, DUTY_50_1000);
            ChangeAmpli(FREQ_800HZ, DUTY_50_800);
            Power_Ampli_Ena();
            siren_state = SIREN_SINGLE_CHOPP_OFF;
            siren_timeout = 1500;					//2 seg
        }
        break;

    case SIREN_SINGLE_CHOPP_OFF:
        if (!siren_timeout)
        {
            //vuelvo y dejo 500ms apagado
            Power_Ampli_Disa();
            siren_state--;
            siren_timeout = 500;
        }
        break;

    case SIREN_SINGLE_CHOPP_SMALL_ON:
        if (!siren_timeout)
        {
            ChangeAmpli(FREQ_800HZ, DUTY_50_800);
            Power_Ampli_Ena();
            siren_state = SIREN_SINGLE_CHOPP_SMALL_OFF;
            siren_timeout = 500;
        }
        break;

    case SIREN_SINGLE_CHOPP_SMALL_OFF:
        if (!siren_timeout)
        {
            Power_Ampli_Disa();
            siren_state--;
            siren_timeout = 1500;
        }
        break;
                        
    case SIREN_MULTIPLE_UP:
        siren_steps = 0;
        siren_state = SIREN_MULTIPLE_UP_B;
        Power_Ampli_Ena();
        break;

    case SIREN_MULTIPLE_UP_A:
        if (!siren_timeout)
        {
            //se termino el tiempo, cambio la frecuencia
            if (siren_steps)
                siren_steps--;
            siren_state = SIREN_MULTIPLE_UP_B;
        }
        break;

    case SIREN_MULTIPLE_UP_B:
        if (siren_steps)
        {
            freq_us = freq_us + SIREN_FREQ_STEP;
            siren_timeout = SIREN_STEP_TIMEOUT;
            ChangeAmpli(freq_us, freq_us >> 1);
            siren_state = SIREN_MULTIPLE_UP_A;
        }
        else
        {
            //empiezo el ciclo de nuevo
            siren_steps = SIREN_STEP_RELOAD;
            freq_us = SIREN_FIRST_FREQ;
            ChangeAmpli(freq_us, freq_us >> 1);
            siren_state = SIREN_MULTIPLE_UP_A;
            siren_timeout = SIREN_STEP_TIMEOUT;
        }
        break;

    case SIREN_CONFIRM_OK:
        siren_steps = 7;
        siren_state = SIREN_CONFIRM_OK_B;
        siren_timeout = SIREN_SHORT_TIMEOUT;
        Power_Ampli_Ena();
        break;

    case SIREN_CONFIRM_OK_A:
        if (!siren_timeout)
        {
            if (siren_steps)
            {
                //se termino el bip
                siren_steps--;
                siren_timeout = SIREN_SHORT_TIMEOUT;
                siren_state = SIREN_CONFIRM_OK_B;
                Power_Ampli_Ena();
            }
            else
            {
                //termino la secuencia
                siren_state = SIREN_TO_STOP;
            }
        }
        break;

    case SIREN_CONFIRM_OK_B:
        if (!siren_timeout)
        {
            //se termino el tiempo
            Power_Ampli_Disa();
            siren_state = SIREN_CONFIRM_OK_A;
            siren_timeout = SIREN_SHORT_TIMEOUT;
        }
        break;

    case SIREN_MULTIPLE_DOWN:
        siren_steps = 0;
        siren_state = SIREN_MULTIPLE_DOWN_B;
        Power_Ampli_Ena();
        break;

    case SIREN_MULTIPLE_DOWN_A:
        if (!siren_timeout)
        {
            //se termino el tiempo, cambio la frecuencia
            if (siren_steps)
                siren_steps--;
            siren_state = SIREN_MULTIPLE_DOWN_B;
        }
        break;

    case SIREN_MULTIPLE_DOWN_B:
        if (siren_steps)
        {
            freq_us = freq_us - SIREN_FREQ_STEP;
            siren_timeout = SIREN_STEP_TIMEOUT;
            ChangeAmpli(freq_us, freq_us >> 1);
            siren_state = SIREN_MULTIPLE_DOWN_A;
        }
        else
        {
            //empiezo el ciclo de nuevo
            siren_steps = SIREN_STEP_RELOAD;
            freq_us = SIREN_FIRST_FREQ;
            ChangeAmpli(freq_us, freq_us >> 1);
            siren_state = SIREN_MULTIPLE_DOWN_A;
            siren_timeout = SIREN_STEP_TIMEOUT;
        }
        break;

    case SIREN_SHORT:
        siren_timeout = SIREN_SHORT_TIMEOUT;
        siren_state = SIREN_SHL_TIMEOUT;
        Power_Ampli_Ena();
        break;

    case SIREN_HALF:
        siren_timeout = SIREN_HALF_TIMEOUT;
        siren_state = SIREN_SHL_TIMEOUT;
        Power_Ampli_Ena();
        break;

    case SIREN_LONG:
        siren_timeout = SIREN_LONG_TIMEOUT;
        siren_state = SIREN_SHL_TIMEOUT;
        Power_Ampli_Ena();
        break;

    case SIREN_SHL_TIMEOUT:
        if (!siren_timeout)
        {
            //se termino el tiempo
            siren_state = SIREN_TO_STOP;
        }
        break;

    case SIREN_TO_STOP:
    default:
        Power_Ampli_Disa();
        siren_state = SIREN_INIT;
        break;
    }
}


void AudioInit (void)
{
    p_numbers_speak = numbers_speak;	//seteo puntero
}
                    
//dice los numero en secuencia
//los numeros se cargan con VectorToSpeak()
void UpdateAudio (void)
{

    switch (audio_state)
    {
    case AUDIO_INIT:
        if (*p_numbers_speak != 0)		//ojo ver esto que no son char
            audio_state++;
        break;

    case AUDIO_UPDATE:
        //habilitar amplificador y pasarlo a audio
        //cargo el audio
        switch (*p_numbers_speak)
        {
        case 10:
            p_files_addr = &files.posi0;
            p_files_length = &files.length0;
            break;

        case 1:
            p_files_addr = &files.posi1;
            p_files_length = &files.length1;
            break;

        case 2:
            p_files_addr = &files.posi2;
            p_files_length = &files.length2;
            break;

        case 3:
            p_files_addr = &files.posi3;
            p_files_length = &files.length3;
            break;

        case 4:
            p_files_addr = &files.posi4;
            p_files_length = &files.length4;
            break;

        case 5:
            p_files_addr = &files.posi5;
            p_files_length = &files.length5;
            break;

        case 6:
            p_files_addr = &files.posi6;
            p_files_length = &files.length6;
            break;

        case 7:
            p_files_addr = &files.posi7;
            p_files_length = &files.length7;
            break;

        case 8:
            p_files_addr = &files.posi8;
            p_files_length = &files.length8;
            break;

        case 9:
            p_files_addr = &files.posi9;
            p_files_length = &files.length9;
            break;

        case 'a':
            p_files_addr = &files.posi10;
            p_files_length = &files.length10;            
            break;

        case 'b':
            p_files_addr = &files.posi11;
            p_files_length = &files.length11;            
            break;
        }

        Load16SamplesShort((unsigned short *)v_samples1, *p_files_addr + FILE_OFFSET);
        Load16SamplesShort((unsigned short *)v_samples2, *p_files_addr + FILE_OFFSET + 32);
        current_size = 64;
        update_samples = 0;

        Power_Ampli_Ena ();
        Ampli_to_Audio ();
        Usart1Send((char *) "-> To Audio\r\n");
#ifdef LED_FOR_AUDIO_TEST_START_FINISH
        Led_On();
#endif
        audio_state++;
        break;

    case AUDIO_SENDING:
        if (update_samples)	//el update lo hace la interrupcion para la funcion seno
        {
            update_samples = 0;

            if (current_size < (*p_files_length - FILE_OFFSET))
            {
                //LED_ON;
                //DESDE ACA LEVANTO DE LA MEMORIA SST
                //cargo el buffer que no esta en uso
                if (buff_in_use == 1)
                {
#ifdef LED_FOR_AUDIO_TEST_EACH_BUFF
                    Led_On();
#endif
                    //cargo el buffer 2
                    Load16SamplesShort((unsigned short *)v_samples2, *p_files_addr + FILE_OFFSET + current_size);
                }
                else if (buff_in_use == 2)
                {
#ifdef LED_FOR_AUDIO_TEST_EACH_BUFF
                    Led_Off();
#endif
                    //cargo el buffer 1
                    Load16SamplesShort((unsigned short *)v_samples1, *p_files_addr + FILE_OFFSET + current_size);
                }
                current_size += 32;
                //LED_OFF;
            }
            else
            {
#ifdef LED_FOR_AUDIO_TEST_START_FINISH
                Led_Off();
#endif
                //termine de enviar avanzo para ver si hay mas numeros
                audio_state++;
            }
        }
        break;

    case AUDIO_CHECK_NEXT:
        p_numbers_speak++;
        if (*p_numbers_speak != '\0')		//ojo ver esto que no son char
            audio_state = AUDIO_UPDATE;
        else
            audio_state = AUDIO_FINISHING;

        break;

    case AUDIO_FINISHING:
        //llegue al final
        p_numbers_speak = numbers_speak;
        memset (numbers_speak, '\0', sizeof(numbers_speak));
        Power_Ampli_Disa ();
        Ampli_to_Sirena ();
        Usart1Send((char *) "-> To Sirena\r\n");
        audio_state = AUDIO_INIT;
        break;

    default:
        audio_state = AUDIO_INIT;
        break;
    }

}


unsigned char Get_Audio_Init (void)
{
    if (audio_state == AUDIO_INIT)
        return 1;

    return 0;
}


//caraga la posicion con VectorToSpeak
void PositionToSpeak(unsigned short posi)
{
    unsigned char a, b, c;

    a = posi / 100;

    if (a == 0)
        VectorToSpeak(10);
    else
        VectorToSpeak(a);

    b = (posi - a * 100) / 10;

    if (b == 0)
        VectorToSpeak(10);
    else
        VectorToSpeak(b);

    c = (posi - a * 100 - b * 10);

    if (c == 0)
        VectorToSpeak(10);
    else
        VectorToSpeak(c);

    VectorToSpeak(0);
}


//carga los numeros a decir en secuencia en un vector
//del 1 al 9; 10 es cero
void VectorToSpeak (unsigned char new_number)
{
    unsigned char i;
    //me fijo si hay espacio
    if (p_numbers_speak < &numbers_speak[LAST_NUMBER_SPEAK])
    {
        //busco la primer posicion vacia y pongo el nuevo numero
        for (i = 0; i < LAST_NUMBER_SPEAK; i++)
        {
            if (numbers_speak[i] == 0)
            {
                numbers_speak[i] = new_number;
                i = LAST_NUMBER_SPEAK;
            }
        }
    }
}


void Siren_Timeouts (void)
{
    if (siren_timeout)
        siren_timeout--;    
}


//--- end of file ---//
