//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### COMMS.H ###############################
//---------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _COMMS_H_
#define _COMMS_H_


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void CommsUpdate (void);
unsigned char UpdateUart(unsigned char last_state);

unsigned char CheckSMS (void);
void ResetSMS (void);
void SetSMS (void);



#endif    /* _COMMS_H_ */

//--- end of file ---//
