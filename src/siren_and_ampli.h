//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SIREN_AND_AMPLI.H ##################################
//----------------------------------------------------------
#ifndef _SIREN_AND_AMPLI_H_
#define _SIREN_AND_AMPLI_H_



// Exported Types Constants and Macros -----------------------------------------

//ESTADOS DE LA SIRENA
#define SIREN_INIT				0
#define SIREN_SINGLE			1
#define SIREN_SINGLE_RINGING	2
#define SIREN_SINGLE_CHOPP_ON	3
#define SIREN_SINGLE_CHOPP_OFF	4
#define SIREN_MULTIPLE_UP		5
#define SIREN_MULTIPLE_UP_A		6
#define SIREN_MULTIPLE_UP_B		7
#define SIREN_TO_STOP			8
#define SIREN_MULTIPLE_DOWN		9
#define SIREN_MULTIPLE_DOWN_A	10
#define SIREN_MULTIPLE_DOWN_B	11
#define SIREN_SHORT				12
#define SIREN_HALF				13
#define SIREN_LONG				14
#define SIREN_SHL_TIMEOUT		15
#define SIREN_CONFIRM_OK		16
#define SIREN_CONFIRM_OK_A		17
#define SIREN_CONFIRM_OK_B		18
#define SIREN_SINGLE_CHOPP_SMALL_ON    20
#define SIREN_SINGLE_CHOPP_SMALL_OFF    21

//COMANDOS DE LA SIRENA	(tienen que ser los del estado de arriba)
#define SIREN_SINGLE_CMD			SIREN_SINGLE
#define SIREN_SINGLE_CHOPP_CMD		SIREN_SINGLE_CHOPP_ON
#define SIREN_STOP_CMD				SIREN_TO_STOP
#define SIREN_MULTIPLE_UP_CMD		SIREN_MULTIPLE_UP
#define SIREN_MULTIPLE_DOWN_CMD		SIREN_MULTIPLE_DOWN
#define SIREN_SHORT_CMD				SIREN_SHORT
#define SIREN_HALF_CMD				SIREN_HALF
#define SIREN_LONG_CMD				SIREN_LONG
#define SIREN_CONFIRM_OK_CMD		SIREN_CONFIRM_OK

#define SIREN_SINGLE_CHOPP_SMALL_CMD    SIREN_SINGLE_CHOPP_SMALL_ON


// Exported Module Functions ---------------------------------------------------
void SirenCommands(unsigned char command);
void UpdateSiren (void);

unsigned char Get_Audio_Init(void);
void AudioInit (void);
void UpdateAudio (void);

void VectorToSpeak (unsigned char new_number);
void PositionToSpeak(unsigned short posi);

void UpdateBattery (void);
void Siren_Timeouts (void);

#endif    /* _SIREN_AND_AMPLI_H_ */

//--- end of file ---//
