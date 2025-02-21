//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### HARD.H #################################
//---------------------------------------------

#ifndef _HARD_H_
#define _HARD_H_

#include "stm32f0xx.h"
// #include "switches_answers.h"


// Compatibility with old version
#define UpdateSwitches(X)    HARD_UpdateSwitches(X)
#define UpdateBuzzer()    BuzzerUpdate()

//-- Defines For Configuration -------------------
//---- Features Configuration ----------------
//----------- Defines For Configuration -------------
// VER HARD.H LO QUE CORRESPONDE A HARDWARE
// VER RWS317.H LO QUE TIENE QUE VER CON CODIGOS Y CONTROLES
#define CON_BLOQUEO_DE_KEYPAD
// #define WITH_WELCOME_CODE_ON_DISPLAY
#define USE_BUTTON4_ONLY_5MINS

// #define USE_F12_PLUS_WITH_SM
#define USE_F12_PLUS_ON_BUTTON1
#define CODES_IN_SST

#define LED_FOR_BATTERY_STATUS
// #define LED_FOR_AUDIO_TEST_START_FINISH
// #define LED_FOR_AUDIO_TEST_EACH_BUFF


#ifdef USE_F12_PLUS_WITH_SM
#define USE_F12_PLUS_SM_DIRECT
// #define USE_F12_PLUS_SM_NEGATE
#endif

//---- Kinf of Program Configuration ----------
#define PROGRAMA_DE_PRODUCCION_Y_GESTION
// #define PROGRAMA_DE_PRODUCCION
// #define PROGRAMA_FACTORY_TEST
// #define PROGRAMA_DE_GESTION
// #define PROGRAMA_PORTON_KIRNO


//---- Subprogram config - Mostly on Production
// #define PRODUCCION_NORMAL
#define PRODUCCION_BUCLE
// #define PRODUCCION_CHICKEN_BUCLE    //boton 1, 2, 3, el 4 apaga

#ifdef PROGRAMA_PORTON_KIRNO
#define WITH_WELCOME_CODE_ON_DISPLAY
// #define USE_KIRNO_CODES    // this or the next
#define USE_MEMB_CODES    // this or the former
#endif

#if defined (PROGRAMA_DE_PRODUCCION) ||\
    defined (PROGRAMA_DE_PRODUCCION_Y_GESTION)
#if (!defined PRODUCCION_NORMAL) &&\
    (!defined PRODUCCION_BUCLE) &&\
    (!defined PRODUCCION_CHICKEN_BUCLE)
#error "Select what kind of production program do you want"
#endif
#endif

//---- Configuration for Hardware and Software Versions -------
#define HARDWARE_VERSION_2_1    // pre-prod 30 units
// #define HARDWARE_VERSION_2_0    // only 5 prototipes

#define SOFTWARE_VERSION_2_5


// -- Sanity Checks For Porton Kirno -----------
#ifdef PROGRAMA_PORTON_KIRNO
#if (defined USE_KIRNO_CODES) && (defined USE_MEMB_CODES)
#error "Select codes for Kirno or Memb, no boths"
#endif
#if (!defined USE_KIRNO_CODES) && (!defined USE_MEMB_CODES)
#error "Select which codes used to work with Kirno or Memb"
#endif
#endif
// -- End of Sanity Checks For Porton Kirno ----

//--- Check the temp sensor ---//
#define USE_OVERTEMP_PROT

//---- End of Features Configuration ----------


//--- Hardware Welcome Code ------------------//
#ifdef HARDWARE_VERSION_2_1
#define HARD "Hardware V: 2.1\r\n"
#define HARD_TO_DISPLAY "h2.1"
#endif

#ifdef HARDWARE_VERSION_2_0
#define HARD "Hardware V: 2.0\r\n"
#define HARD_TO_DISPLAY "h2.0"
#endif

//--- Software Welcome Code ------------------//
#ifdef SOFTWARE_VERSION_2_5
#define SOFT "Software V: 2.5\r\n"
#define SOFT_TO_DISPLAY "s2.5"
#endif

//--- Type of Program Announcement ----------------
#ifdef PRODUCCION_NORMAL
#define KIND_OF_PROGRAM "Produccion - Panel Normal\r\n"
#endif
#ifdef PRODUCCION_BUCLE
#define KIND_OF_PROGRAM "Produccion - Programa para Bucle\r\n"
#endif
#ifdef PRODUCCION_CHICKEN_BUCLE
#define KIND_OF_PROGRAM "Produccion - Bucle Chicken Features\r\n"
#endif
#ifdef PROGRAMA_FACTORY_TEST
#define KIND_OF_PROGRAM "Panel con Programa de testeo en fabrica\r\n"
#endif
#ifdef PROGRAMA_DE_GESTION
#define KIND_OF_PROGRAM "Programa de Gestion para Grabar SST\r\n"
#endif
#ifdef PROGRAMA_PORTON_KIRNO
#define KIND_OF_PROGRAM "Programa del Porton de Kirno\r\n"
#endif

//---- End of Kind of Program Config ----------


//-- Sanity Checks ----------------------------------
#if (!defined HARDWARE_VERSION_2_1) && \
    (!defined HARDWARE_VERSION_2_0)
#error "Not HARD version selected on version.h"
#endif


//-- End of Sanity Checks ---------------------------


#if (defined HARDWARE_VERSION_2_1) || (defined HARDWARE_VERSION_2_0)
//GPIOC pin13
#define CHARGE    ((GPIOC->ODR & 0x2000) != 0)
#define CHARGE_ON    (GPIOC->BSRR = 0x00002000)
#define CHARGE_OFF    (GPIOC->BSRR = 0x20000000)

//GPIOC pin14
//GPIOC pin15    NC

//GPIOF pin0    
//GPIOF pin1    NC

//GPIOC pin0
#define KEY_1    ((GPIOC->IDR & 0x0001) == 0)

//GPIOC pin1
#define KEY_POUND    ((GPIOC->IDR & 0x0002) == 0)

//GPIOC pin2
#define KEY_3    ((GPIOC->IDR & 0x0004) == 0)

//GPIOC pin3
#define KEY_9    ((GPIOC->IDR & 0x0008) == 0)

//GPIOA pin0
#define KEY_6    ((GPIOA->IDR & 0x0001) == 0)

//GPIOA pin1
#define KEY_0    ((GPIOA->IDR & 0x0002) == 0)

//GPIOA pin2
#define KEY_8    ((GPIOA->IDR & 0x0004) == 0)

//GPIOA pin3
#define WP    ((GPIOA->ODR & 0x0008) == 0)
#define WP_OFF    (GPIOA->BSRR = 0x00000008)
#define WP_ON    (GPIOA->BSRR = 0x00080000)


//GPIOF pin4
#define KEY_5    ((GPIOF->IDR & 0x0010) == 0)

//GPIOF pin5
#define KEY_2    ((GPIOF->IDR & 0x0020) == 0)

//GPIOA pin4    NC

//GPIOA pin5
#define KEY_STAR    ((GPIOA->IDR & 0x0020) == 0)

//GPIOA pin6    /CE
#define CE    ((GPIOA->ODR & 0x0040) == 0)
#define CE_OFF    (GPIOA->BSRR = 0x00000040)
#define CE_ON    (GPIOA->BSRR = 0x00400000)


//GPIOA pin7    Alternative TIM1_CH1N or LEFT
#define LEFT    ((GPIOA->ODR & 0x0080) != 0)
#define LEFT_ON    (GPIOA->BSRR = 0x00000080)
#define LEFT_OFF    (GPIOA->BSRR = 0x00800000)

//GPIOC pin4    Adc Sense_NTC

//GPIOC pin5
#define KEY_7    ((GPIOC->IDR & 0x0020) == 0)

//GPIOB pin0    Adc Sense_BAT
//GPIOB pin1    Adc Sense_IN

//GPIOB pin2
#define KEY_4    ((GPIOB->IDR & 0x0004) == 0)

//GPIOB pin10    display seg c
//GPIOB pin11    display seg d
//GPIOB pin12    display seg e
//GPIOB pin13    display seg f
//GPIOB pin14    display seg g
//GPIOB pin15    display seg dp

//GPIOC pin6
#define LED    ((GPIOC->ODR & 0x0040) != 0)
#define LED_ON    (GPIOC->BSRR = 0x00000040)
#define LED_OFF    (GPIOC->BSRR = 0x00400000)

//GPIOC pin7
#define F12PLUS    ((GPIOC->ODR & 0x0080) != 0)
#define F12PLUS_ON    (GPIOC->BSRR = 0x00000080)
#define F12PLUS_OFF    (GPIOC->BSRR = 0x00800000)

//GPIOC pin8
#define FPLUS    ((GPIOC->ODR & 0x0100) != 0)
#define FPLUS_ON    (GPIOC->BSRR = 0x00000100)
#define FPLUS_OFF    (GPIOC->BSRR = 0x01000000)

//GPIOC pin9    Alternative TIM3_CH4
#define RX_CODE    ((GPIOC->IDR & 0x0200) != 0)

//GPIOA pin8    Alternative TIM1_CH1 or RIGHT
#define RIGHT    ((GPIOA->ODR & 0x0100) != 0)
#define RIGHT_ON    (GPIOA->BSRR = 0x00000100)
#define RIGHT_OFF    (GPIOA->BSRR = 0x01000000)

//GPIOA pin9    Alternative USART1_TX
//GPIOA pin10    Alternative USART1_RX

//GPIOA pin11    NC

//GPIOA pin12
#define BUZZER    ((GPIOA->ODR & 0x1000) != 0)
#define BUZZER_ON    (GPIOA->BSRR = 0x00001000)
#define BUZZER_OFF    (GPIOA->BSRR = 0x10000000)

//GPIOA pin13    NC

//GPIOF pin6    
//GPIOF pin7    NC

//GPIOA pin14    NC
//GPIOA pin15    NC

//GPIOC pin10    NC
//GPIOC pin11    NC
//GPIOC pin12    NC

//GPIOD pin2    NC

//GPIOB pin3    
//GPIOB pin4     
//GPIOB pin5    Alternative SPI

//GPIOB pin6    NC
//GPIOB pin7    NC

//GPIOB pin8    display seg b
//GPIOB pin9    display seg a

#endif    //HARDWARE_VERSION_2_1 & HARDWARE_VERSION_2_0


// BUZZER Commands
#define BUZZER_STOP_CMD		0
#define BUZZER_LONG_CMD		1
#define BUZZER_HALF_CMD		2
#define BUZZER_SHORT_CMD	3



// Module Exported Functions ---------------------------------------------------
void HARD_Timeouts (void);
unsigned char HARD_UpdateSwitches (void);
unsigned char HARD_GetSwitches (void);

void BuzzerCommands(unsigned char, unsigned char);
void BuzzerUpdate (void);
unsigned char BuzzerIsFree (void);

unsigned char Led_Is_On (void);
void Led_On (void);
void Led_Off (void);
void Charge_On (void);
void Charge_Off (void);


#endif    /* _HARD_H_ */

//--- end of file ---//
