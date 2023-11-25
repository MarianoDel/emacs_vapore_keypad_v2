//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### BATTERY_AND_CHARGER.H ##############################
//----------------------------------------------------------
#ifndef __BATTERY_AND_CHARGER_H_
#define __BATTERY_AND_CHARGER_H_


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void Battery_Update (void);
void Battery_Timeouts (void);
void Charger_Disconnect_For_Siren (void);
void Charger_Connect (void);


#endif    /* __BATTERY_AND_CHARGER_H */

